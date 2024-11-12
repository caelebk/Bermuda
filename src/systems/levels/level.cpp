#include <numeric>
#include <algorithm>
#include <random>
#include <iterator>
#include <string>

#include "spawning.hpp"
#include "level_spawn.hpp"
#include "map_factories.hpp"
#include "level.hpp"
#include "graph.hpp"
#include "common.hpp"
#include "room.hpp"
#include "player_factories.hpp"

void LevelBuilder::init(RenderSystem* renderer) {
  this->renderer = renderer;
  current_room_id = STARTING_ROOM_ID;

  // TODO: extract magic numbers
  generate_random_level({5, 5, 5}, {80, 50, 0});
};

RoomBuilder& LevelBuilder::room(std::string s_id) {
  return rooms[s_id];
};

HallwayBuilder& LevelBuilder::hallway(std::string s_id) {
  return hallways[s_id];
};

LevelBuilder& LevelBuilder::connect(Direction direction,
                                    std::string r_id,
                                    std::string d1_id,
                                    std::string r2_id,
                                    std::string d2_id) {
  RoomBuilder&    room_1    = rooms[r_id];
  RoomBuilder&    room_2 =  rooms[r2_id];
  Entity&    door_1    = room_1.doors[d1_id];
  Entity&    door_2 =  room_2.doors[d2_id];

  if (!registry.doorConnections.has(door_1) && !registry.doorConnections.has(door_2)) {
    DoorConnection& door_connection_1 = registry.doorConnections.emplace(door_1);
    door_connection_1.direction = direction;
    door_connection_1.room_id = r2_id;
    door_connection_1.exit_door = door_2;

    DoorConnection& door_connection_2 = registry.doorConnections.emplace(door_2);
    door_connection_2.direction = get_opposite_direction(direction);
    door_connection_2.room_id = r_id;
    door_connection_2.exit_door = door_1;
  }

  return *this;
};

void LevelBuilder::activate_boundary(Entity& boundary) {
    Vector &boundary_vector = registry.vectors.get(boundary);
    float boundary_width = abs(boundary_vector.end.x - boundary_vector.start.x);
    float boundary_height = abs(boundary_vector.end.y - boundary_vector.start.y);
    float boundary_pos_x = (boundary_vector.end.x + boundary_vector.start.x) / 2;
    float boundary_pos_y = (boundary_vector.end.y + boundary_vector.start.y) / 2;

    vec2 bounding_box;
    if (boundary_width <= 0) {
      bounding_box = vec2(WALL_THICKNESS, boundary_height + WALL_THICKNESS);
    } else if (boundary_height <= 0) {
      bounding_box = vec2(boundary_width + WALL_THICKNESS, WALL_THICKNESS);
    }
    vec2 position =
        vec2(boundary_pos_x + ROOM_ORIGIN_POS.x, boundary_pos_y + ROOM_ORIGIN_POS.y);

    // Setting initial position values
    Position &position_component = registry.positions.emplace(boundary);
    position_component.position = position;
    position_component.angle = 0.f;
    position_component.scale = bounding_box;
}

void LevelBuilder::deactivate_boundary(Entity& boundary) {
  if (registry.positions.has(boundary)) {
    registry.positions.remove(boundary);
  }
  if (registry.renderRequests.has(boundary)) {
    registry.renderRequests.remove(boundary);
  }
  if (registry.activeWalls.has(boundary)) {
    registry.activeWalls.remove(boundary);
  }
  if (registry.activeDoors.has(boundary)) {
    registry.activeDoors.remove(boundary);
  }
};

void LevelBuilder::mark_boss_rooms(std::vector<int> rooms) {
  int i = 0;
  int cluster = -1;
  while (i < rooms.size()) {
    cluster += rooms[i];
    boss_rooms.insert(std::to_string(cluster));
    i++;
  }

  for (auto& boss_room : boss_rooms) {
    std::cout << "boss room: " << boss_room << std::endl;
  }
}

void LevelBuilder::spawn_miniboss() {
  execute_config_rand(CRAB_MINIBOSS, room(current_room_id), renderer); // Replace "LVL_1_RAND_POS" with the macro for miniboss.
  std::cout << "Miniboss spawned!" << std::endl;
}

void LevelBuilder::mark_room_entered(std::string room_id) {
  entered_rooms.insert(room_id);
}

bool LevelBuilder::has_entered_room(std::string room_id) {
  return entered_rooms.count(room_id) > 0;
}

void LevelBuilder::activate_room(std::string room_id) {
  current_room_id = room_id;

  // Render the walls.
  for (Entity& wall : registry.spaces.get(room(current_room_id).entity).walls) {
    activate_boundary(wall);
    registry.activeWalls.emplace(wall);
    registry.renderRequests.insert(wall, {TEXTURE_ASSET_ID::WALL,
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});
  }

  // Render the doors.
  for (Entity& door : registry.spaces.get(room(current_room_id).entity).doors) {
    activate_boundary(door);
    registry.activeDoors.emplace(door);
    Direction direction = registry.doorConnections.get(door).direction;
    Position& door_position = registry.positions.get(door);
    if (direction == Direction::SOUTH || direction == Direction::WEST) {
      door_position.angle = M_PI;
    }
    if (direction == Direction::NORTH || direction == Direction::SOUTH) {
      registry.renderRequests.insert(door, {TEXTURE_ASSET_ID::DOORWAY_H,
                                            EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});
    } else {
      registry.renderRequests.insert(door, {TEXTURE_ASSET_ID::DOORWAY_V,
                                            EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});
    }
  }

  // activate the floor
  Entity floor = Entity();
  Position &floor_pos = registry.positions.emplace(floor);
  floor_pos.position = {window_width_px / 2, window_height_px / 2};
  floor_pos.scale = {window_width_px, window_height_px};

  registry.floors.emplace(floor);

  registry.renderRequests.insert(floor, {TEXTURE_ASSET_ID::FLOOR,
                                        EFFECT_ASSET_ID::TEXTURED,
                                        GEOMETRY_BUFFER_ID::SPRITE});

// Spawn the appropriate entities.
  // TODO: extract to function
  // If we're in a boss room, just only spawn the boss; ignore everything else.
  if (boss_rooms.count(current_room_id)) {
    spawn_miniboss();
  } else if (!has_entered_room(current_room_id)) {
    mark_room_entered(current_room_id);
    // It's the first time in this room, spawn enemies AND crates.
    execute_config_rand(LVL_1_RAND_POS, room(room_id), renderer);
    execute_config_rand_chance(LVL_1_RAND_POS, room(room_id), renderer, 0.5);

    // Also, store the positions of every crate for a later visit to this room.
    ComponentContainer<Deadly>& enemy_container = registry.deadlys;
    ComponentContainer<ActiveWall>& wall_container = registry.activeWalls;
    for (uint i = 0; i < wall_container.components.size(); i++) {
      Entity entity_i = wall_container.entities[i];
      if (registry.deadlys.has(entity_i)) {
        if (!registry.positions.has(entity_i)) {
          continue;
        }
        Position& position = registry.positions.get(entity_i);
        // TODO: this is a bit of a hack, crates are enemies that are walls
        crate_positions[current_room_id].push_back(position.position);
      }
    }
  } else {
    // Retrieve all the stored crate positions and stick 'em on the floor.
    for (auto& crate_position : crate_positions[current_room_id]) { 
      createCratePos(renderer, crate_position);
    }

    execute_config_rand(LVL_1_RAND_POS_CRATELESS, room(current_room_id), renderer);
    execute_config_rand_chance(LVL_1_RAND_POS_CRATELESS, room(current_room_id), renderer, 0.5);
  }
}

void LevelBuilder::deactivate_room() {
  for (auto &boundary : registry.spaces.get(room(current_room_id).entity).boundaries) {
    deactivate_boundary(boundary);
  }

  // Remove every geyser on the map.
  for (auto& geyser : registry.geysers.entities) {
    registry.remove_all_components_of(geyser);
  }

  // Also, remove every enemy on the map.
  for (auto& enemy : registry.deadlys.entities) {
    registry.remove_all_components_of(enemy);
  }

  // And every consumable as well.
  for (auto& consumable : registry.consumables.entities) {
    registry.remove_all_components_of(consumable);
  }
}

void LevelBuilder::move_player_to_door(Direction direction, Entity& exit_door) {
  for (auto &door : registry.spaces.get(room(current_room_id).entity).doors) {
    if (door == exit_door) {
      if (registry.positions.has(door)) {
        Position& door_position = registry.positions.get(door);
        for (auto &player : registry.players.entities) {
          Position& player_position = registry.positions.get(player);

          // Offset the player from the door so they don't immediately reswitch rooms.
          // Get the opposite direction of the wall that this door was on; i.e if you enter a door on the WEST, you should spawn on the EAST.
          player_position.position = door_position.position;
          Direction opposite_direction = get_opposite_direction(direction);
          
          float offset = PLAYER_BOUNDING_BOX.x * PLAYER_SCALE_FACTOR.x;
          switch(opposite_direction) {
            case Direction::NORTH:
              player_position.position.y += offset;
              break;
            case Direction::EAST:
              player_position.position.x -= offset;
              break;
            case Direction::SOUTH:
              player_position.position.y -= offset;
              break;
            case Direction::WEST:
              player_position.position.x += offset;
              break;
          }
          Player& player_comp = registry.players.get(player);
          Position& player_mesh_pos = registry.positions.get(player_comp.collisionMesh);
          player_mesh_pos.position = player_position.position;
        }
      }
    } 
  }
}

void LevelBuilder::build_wall_with_random_doors(RoomBuilder& room,
                                        Direction direction,
                                        std::unordered_map<int, Direction> directed_adjacencies,
                                        int max_units,
                                        int unit_size,
                                        std::function<void(int)> draw_segment,
                                        std::function<void(std::string, int)> draw_door) {
  int door_index = 0; 
  int current_size = 0;
  std::vector<int> connected_rooms = get_connecting_vertices_with_direction(direction, directed_adjacencies);
  std::vector<int> door_positions = get_random_door_positions(connected_rooms.size(), 0, max_units);
  while (door_index < (int) door_positions.size() && current_size < max_units) {
      int current_door = door_positions[door_index];
      int other_room = connected_rooms[door_index];

      // Calculate the segment before the next door position
      int segment = current_door - current_size;

      draw_segment(unit_size * segment);
      // Generate a door with the id: <direction_enum>_<room_number>; i.e NORTH to room 1 is 0_1.
      std::string door_id;
      door_id.append(std::to_string(other_room)).append("_").append(std::to_string(direction));
      draw_door(door_id, DOOR_SCALAR * unit_size);

      current_size += segment + DOOR_SCALAR; // Update for door and segment size
      door_index++;
  }

  // Fill the remaining wall segment if needed
  if (current_size < max_units) {
      int remaining_segment = max_units - current_size;
      draw_segment(unit_size * remaining_segment);
  }
};

void LevelBuilder::randomize_room_shapes(std::unordered_map<int, std::unordered_map<int, Direction>>& adjacency_list) {
  for (const auto& pair : adjacency_list) {
    std::string room_number = std::to_string(pair.first);
    std::unordered_map<int, Direction> directed_adjacencies = pair.second;

    RoomBuilder& current_room = room(room_number);
    
    // west
    build_wall_with_random_doors(current_room, Direction::WEST, directed_adjacencies, MAX_Y_UNITS, Y_1U,
        [&](int segment) { current_room.up(segment); },
        [&](std::string door_id, int door_size) { current_room.door(door_id, door_size); });

    // north
    build_wall_with_random_doors(current_room, Direction::SOUTH, directed_adjacencies, MAX_X_UNITS, X_1U,
        [&](int segment) { current_room.right(segment); },
        [&](std::string door_id, int door_size) { current_room.door(door_id, door_size); });

    // east
    build_wall_with_random_doors(current_room, Direction::EAST, directed_adjacencies, MAX_Y_UNITS, Y_1U,
        [&](int segment) { current_room.down(segment); },
        [&](std::string door_id, int door_size) { current_room.door(door_id, door_size); });

    // south
    build_wall_with_random_doors(current_room, Direction::NORTH, directed_adjacencies, MAX_X_UNITS, X_1U,
        [&](int segment) { current_room.left(segment); },
        [&](std::string door_id, int door_size) { current_room.door(door_id, door_size); });
  }
}

void LevelBuilder::connect_doors(std::unordered_map<int, std::unordered_map<int, Direction>>& adjacency_list) {
  for (const auto& adjacency_list_pair : adjacency_list) {
    std::string r1 = std::to_string(adjacency_list_pair.first);
    std::unordered_map<int, Direction> directed_adjacencies = adjacency_list_pair.second;

    for (const auto& adjacency_pair : directed_adjacencies) {
      std::string r2 = std::to_string(adjacency_pair.first);
      Direction direction = adjacency_pair.second;
      std::string d1;
      d1.append(r2).append("_").append(std::to_string(direction));
      std::string d2;
      d2.append(r1).append("_").append(std::to_string(get_opposite_direction(direction)));

      connect(direction, r1, d1, r2, d2);
    }
  }
}

// Note since we transition when moving spaces, these graphs don't have to be planar.
void LevelBuilder::generate_random_level(std::vector<int> rooms, std::vector<int> densities) {
  // Generate a randomized graph with constraints. We then ECS-ify its vertices and edges into rooms and connections.
  std::unordered_map<int, std::set<int>> random_graph = generate_random_graph(rooms, densities);
  // Pass over that generated graph, greedily and randomly associating each edge (door) with a direction. Since the graph is undirected, do the same for the exit door, with
  // the opposite direction.
  std::unordered_map<int, std::unordered_map<int, Direction>> random_graph_with_doors = expand_graph_to_random_directed_graph(random_graph);

  // Now, we can turn the graph with directions for each door into ECS entities and components.
  // First, randomize the placement of each door in each room such that it maps to the direction we assigned. Add all created entities to the ECS.
  randomize_room_shapes(random_graph_with_doors);
  // Now that all the doors are in the ECS, add their connections to the ECS.
  connect_doors(random_graph_with_doors);
  // Also, mark each boss room.
  mark_boss_rooms(rooms);
};

void LevelBuilder::enter_room(DoorConnection& door_connection) {
  Direction direction = door_connection.direction;
  std::string room_id = door_connection.room_id;
  Entity& exit_door = door_connection.exit_door;

  deactivate_room();
  activate_room(room_id);
  move_player_to_door(direction, exit_door);

  std::cout << "Player entered room: " << current_room_id << std::endl;
}

void LevelBuilder::activate_starting_room() {
  deactivate_room();
  activate_room(STARTING_ROOM_ID);
}
