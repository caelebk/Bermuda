#include "level_builder.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>

#include "graph.hpp"
#include "level_spawn.hpp"
#include "room_builder.hpp"
#include "map_factories.hpp"
#include "player_factories.hpp"
#include "spawning.hpp"
#include "level_factories.hpp"

RoomBuilder& LevelBuilder::get_room_by_editor_id(std::string s_id) {
  return rooms[s_id];
};

LevelBuilder& LevelBuilder::connect(Direction direction, std::string r_id,
                                    std::string d1_id, std::string r2_id,
                                    std::string d2_id) {
  RoomBuilder& room_1 = rooms[r_id];
  RoomBuilder& room_2 = rooms[r2_id];
  Entity&      door_1 = room_1.doors[d1_id];
  Entity&      door_2 = room_2.doors[d2_id];

  if (!registry.doorConnections.has(door_1) &&
      !registry.doorConnections.has(door_2)) {
    DoorConnection& door_connection_1 =
        registry.doorConnections.emplace(door_1);
    door_connection_1.direction = direction;
    door_connection_1.room_id   = r2_id;
    door_connection_1.exit_door = door_2;

    DoorConnection& door_connection_2 =
        registry.doorConnections.emplace(door_2);
    door_connection_2.direction = get_opposite_direction(direction);
    door_connection_2.room_id   = r_id;
    door_connection_2.exit_door = door_1;
  }

  return *this;
};

void LevelBuilder::mark_difficulty_regions() {
  assert(ROOM_CLUSTERS.size() == ROOM_CLUSTER_SPAWN_FUNCTIONS.size());
  
  int room_index = 0;

  for (unsigned int i = 0; i < ROOM_CLUSTERS.size(); i++) {
    int rooms_in_cluster = ROOM_CLUSTERS[i];
    
    // Assign the current spawn function to all rooms in the current cluster,
    for (int j = 0; j < rooms_in_cluster; j++) {
      rooms[std::to_string(room_index)].room_spawn_function = ROOM_CLUSTER_SPAWN_FUNCTIONS[i];
      room_index++;
    }
  }
}

void LevelBuilder::mark_tutorial_room() {
  rooms[std::to_string(TUTORIAL_ROOM)].is_tutorial_room = true;
}

void LevelBuilder::mark_miniboss_rooms() {
  // TODO:
  // Move this to a preprocessing check that just does sanity checks on all inputs to make sure you don't break it.
  assert(MINIBOSS_ROOMS.size() == MINIBOSS_SPAWN_FUNCTIONS.size());
  for (unsigned int i = 0; i < MINIBOSS_ROOMS.size(); i++) {
    rooms[std::to_string(MINIBOSS_ROOMS[i])].is_boss_room = true;
    rooms[std::to_string(MINIBOSS_ROOMS[i])].boss_spawn_function = MINIBOSS_SPAWN_FUNCTIONS[i];
  }
}

void LevelBuilder::mark_final_boss_room() {
  rooms[std::to_string(FINAL_BOSS_ROOM)].is_boss_room = true;
}

void LevelBuilder::build_wall_with_random_doors(
    Direction direction,
    std::unordered_map<int, Direction> directed_adjacencies, int max_units,
    int unit_size, std::function<void(int)> draw_segment,
    std::function<void(std::string, int)> draw_door) {
    int door_index   = 0;
    int current_size = 0;
  std::vector<int> connected_rooms =
      get_connecting_vertices_with_direction(direction, directed_adjacencies);
  std::vector<int> door_positions =
      get_random_door_positions(connected_rooms.size(), 0, max_units);
  while (door_index < (int)door_positions.size() && current_size < max_units) {
    int current_door = door_positions[door_index];
    int other_room   = connected_rooms[door_index];

    // Calculate the segment before the next door position
    int segment = current_door - current_size;

    draw_segment(unit_size * segment);
    // Generate a door with the id: <direction_enum>_<room_number>; i.e NORTH to
    // room 1 is 0_1.
    std::string door_id;
    door_id.append(std::to_string(other_room))
        .append("_")
        .append(std::to_string(direction));
    draw_door(door_id, DOOR_SIZE * unit_size);

    current_size += segment + DOOR_SIZE;  // Update for door and segment size
    door_index++;
  }

  // Fill the remaining wall segment if needed
  if (current_size < max_units) {
    int remaining_segment = max_units - current_size;
    draw_segment(unit_size * remaining_segment);
  }
};

void LevelBuilder::randomize_room_shapes(
    std::unordered_map<int, std::unordered_map<int, Direction>>&
        adjacency_list) {
  for (const auto& pair : adjacency_list) {
    std::string                        room_number = std::to_string(pair.first);
    std::unordered_map<int, Direction> directed_adjacencies = pair.second;

    RoomBuilder& current_room = get_room_by_editor_id(room_number);

    // west
    build_wall_with_random_doors(
        Direction::WEST, directed_adjacencies, MAX_Y_UNITS, Y_1U,
        [&](int segment) { current_room.up(segment); },
        [&](std::string door_id, int door_size) {
          current_room.door(door_id, door_size);
        });

    // north
    build_wall_with_random_doors(
        Direction::SOUTH, directed_adjacencies, MAX_X_UNITS, X_1U,
        [&](int segment) { current_room.right(segment); },
        [&](std::string door_id, int door_size) {
          current_room.door(door_id, door_size);
        });

    // east
    build_wall_with_random_doors(
        Direction::EAST, directed_adjacencies, MAX_Y_UNITS, Y_1U,
        [&](int segment) { current_room.down(segment); },
        [&](std::string door_id, int door_size) {
          current_room.door(door_id, door_size);
        });

    // south
    build_wall_with_random_doors(
        Direction::NORTH, directed_adjacencies, MAX_X_UNITS, X_1U,
        [&](int segment) { current_room.left(segment); },
        [&](std::string door_id, int door_size) {
          current_room.door(door_id, door_size);
        });
  }
}

void LevelBuilder::connect_doors(
    std::unordered_map<int, std::unordered_map<int, Direction>>&
        adjacency_list) {
  for (const auto& adjacency_list_pair : adjacency_list) {
    std::string r1 = std::to_string(adjacency_list_pair.first);
    std::unordered_map<int, Direction> directed_adjacencies =
        adjacency_list_pair.second;

    for (const auto& adjacency_pair : directed_adjacencies) {
      std::string r2        = std::to_string(adjacency_pair.first);
      Direction   direction = adjacency_pair.second;
      std::string d1;
      d1.append(r2).append("_").append(std::to_string(direction));
      std::string d2;
      d2.append(r1).append("_").append(
          std::to_string(get_opposite_direction(direction)));

      connect(direction, r1, d1, r2, d2);
    }
  }
}

// Note since we transition when moving spaces, these graphs don't have to be
// planar.
void LevelBuilder::generate_random_level() {
  // Generate a randomized graph with constraints. We then ECS-ify its vertices
  // and edges into rooms and connections.
  std::unordered_map<int, std::set<int>> random_graph =
      generate_random_graph();
  // Pass over that generated graph, greedily and randomly associating each edge
  // (door) with a direction. Since the graph is undirected, do the same for the
  // exit door, with the opposite direction.
  std::unordered_map<int, std::unordered_map<int, Direction>>
      random_graph_with_doors =
          expand_graph_to_random_directed_graph(random_graph);

  // Now, we can turn the graph with directions for each door into ECS entities
  // and components. First, randomize the placement of each door in each room
  // such that it maps to the direction we assigned. Add all created entities to
  // the ECS.
  randomize_room_shapes(random_graph_with_doors);
  // Now that all the doors are in the ECS, add their connections to the ECS.
  connect_doors(random_graph_with_doors);
  
  // Also, mark important rooms.
  mark_difficulty_regions();
  mark_tutorial_room();
  mark_miniboss_rooms();
  mark_final_boss_room();
}
void LevelBuilder::mark_all_rooms_unvisited() {
  for (const auto& pair : rooms) {
    get_room_by_editor_id(pair.first).has_entered = false;
  }
};
