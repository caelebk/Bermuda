#include <numeric>
#include <algorithm>
#include <random>
#include <iterator>
#include <string>

#include "level.hpp"

#include "common.hpp"
#include "room.hpp"

#define MIN_CONNECTIONS 1
#define MAX_CONNECTIONS 4

int get_random(int x, int y) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribution(x, y);
    return distribution(gen);
}

Direction get_opposite_direction(Direction direction) {
  Direction opposite;
  switch(direction) {
    case NORTH:
      opposite = SOUTH;
      break;
    case EAST:
      opposite = WEST;
      break;
    case SOUTH:
      opposite = NORTH;
      break;
    case WEST:
      opposite = EAST;
      break;
  }
  return opposite;
};

LevelBuilder::LevelBuilder(){};

int LevelBuilder::count_directions_of_directed_adjacencies(Direction direction, std::unordered_map<int, Direction>& directed_adjacencies) {
  int count = 0;
  for (const auto& pair : directed_adjacencies) {
      if (pair.second == direction) {
        count++;
      }
  }
  return count;
}

std::vector<int> LevelBuilder::get_connected_rooms_matching_direction(
    Direction                           direction,
    std::unordered_map<int, Direction>& directed_adjacencies) {
  std::vector<int> connected_rooms;

  for (const auto& pair : directed_adjacencies) {
    int       other_room     = pair.first;
    Direction pair_direction = pair.second;
    if (direction == pair_direction) {
      connected_rooms.push_back(other_room);
    }
  }

  return connected_rooms;
}

std::vector<int> LevelBuilder::get_random_door_positions(std::vector<int>& connected_rooms, int min, int max) {
  std::vector<int> positions;

  // Count the number of elements with the specified direction
  int count = connected_rooms.size();

  // Generate unique positions with a minimum distance of 2 between them
  while ((int)positions.size() < count) {
    int  pos      = get_random(min, max);
    bool is_valid = true;

    // Ensure the position is at least 2 units away from both min and max
    if (pos <= min + DOOR_SCALAR || pos >= max - DOOR_SCALAR) {
      is_valid = false;
    }

    // Ensure each new position is at least 2 units away from existing ones
    for (int existing_pos : positions) {
      if (std::abs(existing_pos - pos) < DOOR_SCALAR) {
        is_valid = false;
        break;
      }
    }

    if (is_valid) {
      positions.push_back(pos);
    } else {
      positions = std::vector<int>();
    }
  }

  std::sort(positions.begin(), positions.end());
  return positions;
}

RoomBuilder& LevelBuilder::room(std::string s_id) {
  return rooms[s_id];
};

HallwayBuilder& LevelBuilder::hallway(std::string s_id) {
  return hallways[s_id];
};

void LevelBuilder::connect(Entity& connectee, Entity& connector) {
  registry.adjacencies.get(connectee).neighbours.push_back(connector);
};

LevelBuilder& LevelBuilder::connect_rooms(std::string r_id,
                                          std::string d1_id,
                                          std::string r2_id,
                                          std::string d2_id) {
  RoomBuilder&    room    = rooms[r_id];
  RoomBuilder&    room_2 =  rooms[r2_id];

  connect(room.entity, room_2.entity);
  connect(room.doors[d1_id], room_2.doors[d2_id]);
  connect(room_2.doors[d2_id], room.doors[d1_id]);
  connect(room_2.entity, room.entity);
  return *this;
};

RoomBuilder LevelBuilder::copy_room(std::string s_id, std::string copied_s_id) {
  room(s_id);
  registry.spaces.get(room(s_id).entity) =
      registry.spaces.get(room(copied_s_id).entity);
  registry.bounding_boxes.get(room(s_id).entity) =
      registry.bounding_boxes.get(room(copied_s_id).entity);
  return rooms[s_id];
};

void LevelBuilder::build_wall_with_doors(RoomBuilder& room,
                                        Direction direction,
                                        std::unordered_map<int, Direction> directed_adjacencies,
                                        int max_units,
                                        int unit_size,
                                        std::function<void(int)> draw_segment,
                                        std::function<void(std::string, int)> draw_door) {
  int door_index = 0; 
  int current_size = 0;
  std::vector<int> connected_rooms = get_connected_rooms_matching_direction(direction, directed_adjacencies);
  std::vector<int> door_positions = get_random_door_positions(connected_rooms, 0, max_units);
  while (door_index < (int) door_positions.size() && current_size < max_units) {
      int current_door = door_positions[door_index];
      int other_room = connected_rooms[door_index];

      // Calculate the segment before the next door position
      int segment = current_door - current_size;

      draw_segment(unit_size * segment);
      // Generate a door with the id: <direction_enum>_<room_number>; i.e NORTH to room 1 is 0_1.
      std::string door_id;
      door_id.append(std::to_string(other_room)).append("_").append(std::to_string(direction));
      draw_door(door_id, 2 * unit_size);

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

    // Make a room.
    RoomBuilder& current_room = room(room_number);
    
    // west
    build_wall_with_doors(current_room, WEST, directed_adjacencies, MAX_Y_UNITS, Y_1U,
        [&](int segment) { current_room.up(segment); },
        [&](std::string door_id, int door_size) { current_room.door(door_id, door_size); });

    // north
    build_wall_with_doors(current_room, SOUTH, directed_adjacencies, MAX_X_UNITS, X_1U,
        [&](int segment) { current_room.right(segment); },
        [&](std::string door_id, int door_size) { current_room.door(door_id, door_size); });

    // east
    build_wall_with_doors(current_room, EAST, directed_adjacencies, MAX_Y_UNITS, Y_1U,
        [&](int segment) { current_room.down(segment); },
        [&](std::string door_id, int door_size) { current_room.door(door_id, door_size); });

    // south
    build_wall_with_doors(current_room, NORTH, directed_adjacencies, MAX_X_UNITS, X_1U,
        [&](int segment) { current_room.left(segment); },
        [&](std::string door_id, int door_size) { current_room.door(door_id, door_size); });
  }
}

std::unordered_map<int, std::unordered_map<int, Direction>> LevelBuilder::generate_random_graph_doors(std::unordered_map<int, std::set<int>>& adjacency_list) {
  std::random_device rd;
  std::default_random_engine rng(rd());
  std::unordered_map<int, std::unordered_map<int, Direction>> directed_adjacency_list;
  std::set<std::pair<int, int>> processed_pairs;

  std::vector<Direction> directions = {NORTH, EAST, SOUTH, WEST};
  for (const auto& pair : adjacency_list) {
    int room_number = pair.first;
    std::vector<int> edges(pair.second.begin(), pair.second.end());

    for (int i = 0; i < (int) edges.size(); i++) {
      int other_room = edges[i];

      Direction random_direction;
      do {
        random_direction = directions[get_random(0, directions.size() - 1)];
      } while (
        count_directions_of_directed_adjacencies(random_direction, directed_adjacency_list[room_number]) >= 3
      );

      // Skip if already processed in reverse
      if (processed_pairs.count({room_number, other_room}) || processed_pairs.count({other_room, room_number})) {
        continue;
      }

      directed_adjacency_list[room_number][other_room] = random_direction;

      // Set opposite direction for the other room
      Direction opposite_direction = get_opposite_direction(random_direction);
      directed_adjacency_list[other_room][room_number] = opposite_direction;

      // Mark this pair as processed
      processed_pairs.insert({room_number, other_room});
    }
  }

  // Debugging output
  std::cout << "Adjacency List with Door Connections" << std::endl;
  for (int room_number = 0; room_number < (int) directed_adjacency_list.size(); room_number++) {
    std::unordered_map<int, Direction>& directions = directed_adjacency_list[room_number];
    std::cout << "room " << room_number << ": ";
    for (auto const& direction_pair : directions) {
      int other_room = direction_pair.first;
      Direction direction = direction_pair.second;
      switch(direction) {
        case NORTH: std::cout << "(NORTH," << other_room << ") "; break;
        case EAST: std::cout << "(EAST," << other_room << ") "; break;
        case SOUTH: std::cout << "(SOUTH," << other_room << ") "; break;
        case WEST: std::cout << "(WEST," << other_room << ") "; break;
      }
    }
    std::cout << "\n";
  }

  return directed_adjacency_list;
}

std::unordered_map<int, std::set<int>> LevelBuilder::generate_random_graph(std::vector<int>& rooms, std::vector<int>& densities) {
  // Make a non-ECS adjacency list as our workspace. This will be ECS-ified once we finish the graph.
  std::unordered_map<int, std::set<int>> adjacency_list;
    
  int total_rooms = 0;
  for (int n : rooms) {
    total_rooms += n;
  }
  std::random_device rd;
  std::default_random_engine rng(rd()); 

  int first_room_number = 0;
  for (int cluster = 0; cluster < (int) rooms.size(); cluster++) {
    int rooms_in_cluster = rooms[cluster];
    int p = densities[cluster];
    // Generate a random path through this cluster. random_path also doubles as a normalized access of 0->cluster indices to 
    // the random path, for later loops.
    std::vector<int> random_path = std::vector<int>(rooms_in_cluster);
    std::iota(random_path.begin(), random_path.end(), first_room_number);
    // Keep the first and last nodes fixed, so we can connect difficulty areas via first_room and the elements of the input vector.
    std::shuffle(random_path.begin() + 1, random_path.end() - 1, rng);

    // Fill in the adjacency list for that path.
    for (int i = 0; i < rooms_in_cluster; i++) {
      int room_number = random_path[i];
      if (i == 0) {
        adjacency_list[room_number].insert(random_path[i+1]);
      } else if (i == rooms_in_cluster - 1) {
        adjacency_list[room_number].insert(random_path[i-1]);
      } else {
        adjacency_list[room_number].insert(random_path[i+1]);
        adjacency_list[room_number].insert(random_path[i-1]);
      }
    }

    for (int i = 0; i < rooms_in_cluster; i++) {
      int room_number = random_path[i];
      // Factor in the density probability and check that the room isn't the last one or already has the maximum number of connections.
      if (get_random(1, 100) < p || room_number == random_path.back() || adjacency_list[room_number].size() >= MAX_CONNECTIONS) {
        continue;
      }

      // Connect this room to up to at most four other rooms in total.
      int num_connections = get_random(0, MAX_CONNECTIONS - adjacency_list[room_number].size());
      while (num_connections > 0) {
        int other_room_number;
        do {
          other_room_number = random_path[get_random(0, rooms_in_cluster - 1)];
          // Keep selecting another room in this cluster until it fulfills all of:
          // 1. It is not equal to the connectee room.
          // 2. It is not already connected to the connectee room.
          // 3. It does not yet have the maximum number of connections.
        } while (
          other_room_number == room_number
          || adjacency_list[room_number].count(other_room_number) != 0
          || adjacency_list[other_room_number].size() >= MAX_CONNECTIONS
        );

        // Leave the exit room as a funnel point so we can place a mini-boss there. Also leave at least one connection to delineate map difficulty transitions.
        if (
          other_room_number == random_path.back()
          || (other_room_number == first_room_number && adjacency_list[first_room_number].size() >= MAX_CONNECTIONS - 1)
          ) {
          num_connections--;
          continue;
        }

        // Add the connection.
        adjacency_list[room_number].insert(other_room_number);
        adjacency_list[other_room_number].insert(room_number);

        // decrement num_connections
        num_connections--;
      }
    }

    // Mark the first room that begins a new difficulty area.
    first_room_number += rooms[cluster];
  }

  // Connect all the transitory rooms together that delineate a new difficulty area. The transitory rooms are guaranteed to have available connections.
  int transition_room = rooms.front() - 1;
  for (int i = 0; i < (int) rooms.size() - 1; i++) {
    adjacency_list[transition_room].insert(transition_room+1);
    adjacency_list[transition_room+1].insert(transition_room);
    transition_room += rooms[i];
  }

  // debug printing 
  std::cout << "Adjacency List" << std::endl;
  for (int i = 0; i < total_rooms; i++) {
    std::cout << "room " << i << ": ";
    for (int j : adjacency_list[i]) {
      std::cout << j << " ";
    }
    std::cout << "\n";
  }

  return adjacency_list;
};

void LevelBuilder::connect_doors(std::unordered_map<int, std::unordered_map<int, Direction>>& adjacency_list) {
  for (const auto& adjacency_list_pair : adjacency_list) {
    std::string r1 = std::to_string(adjacency_list_pair.first);
    std::unordered_map<int, Direction> directed_adjacencies = adjacency_list_pair.second;

    // Do this by iterating over directed adjacencies for each room_number, and connecting the corresponding door to the one at adjacency_list[other_room][room_number].
    for (const auto& adjacency_pair : directed_adjacencies) {
      std::string r2 = std::to_string(adjacency_pair.first);
      Direction direction = adjacency_pair.second;
      std::string d1;
      d1.append(r2).append("_").append(std::to_string(direction));
      std::string d2;
      d2.append(r1).append("_").append(std::to_string(get_opposite_direction(direction)));

      connect_rooms(r1, d1, r2, d2);
    }
  }
}

// Note since we transition when moving spaces, these graphs don't have to be planar.
void LevelBuilder::generate_random_level(std::vector<int> rooms, std::vector<int> densities) {
  // Generate a randomized graph with constraints so we can pass over the graph again and ECS-ify its vertices and edges into rooms and connections.
  std::unordered_map<int, std::set<int>> random_graph = generate_random_graph(rooms, densities);
  // Pass over the generated graph, greedily and randomly associating each edge (door) with a direction. Since the graph is undirected, do the same for the exit door, with
  // the opposite direction.
  std::unordered_map<int, std::unordered_map<int, Direction>> random_graph_with_doors = generate_random_graph_doors(random_graph);

  // Now, we can turn the graph with directions for each door into ECS entities and components.
  // First, randomize the placement of each door in each room such that it maps to the direction we assigned; i.e if we have assigned EAST to a door in the graph generation
  // phase, it should be somewhere on the left wall. Add all created entities to the ECS.
  randomize_room_shapes(random_graph_with_doors);
  // Now that all the doors are in place, pass over each door and connect them in the ECS.
  connect_doors(random_graph_with_doors);

  // print_rooms();
};

void LevelBuilder::buildRoomOne() {
  room(ROOM_ONE).up(Y_10U).right(X_14U).down(Y_5U).right(X_6U).down(Y_5U).left(
      X_20U);
}

void LevelBuilder::print_pair(std::pair<std::string, Entity> pair) {
  std::cout << "==================" << std::endl;
  std::cout << "  key:" << pair.first << std::endl;
  std::cout << "  id:" << pair.second << std::endl;
  Space& space = registry.spaces.get(pair.second);
  std::cout << "  walls: " << std::endl;
  for (auto& wall : space.walls) {
    Vector& vector = registry.vectors.get(wall);
    printf("    (%f, %f), (%f, %f):\n", vector.start.x, vector.start.y,
           vector.end.x, vector.end.y);
  }
  std::cout << "  doors:" << std::endl;
  for (auto& door : space.doors) {
    Vector& vector = registry.vectors.get(door);
    printf("    (%f, %f), (%f, %f):\n", vector.start.x, vector.start.y,
           vector.end.x, vector.end.y);
  }
  std::cout << "  connections:" << std::endl;
  Adjacency& adjacencies = registry.adjacencies.get(pair.second);
  for (auto& neighbour : adjacencies.neighbours) {
    std::cout << "    " << neighbour << std::endl;
  }
}

void LevelBuilder::print_rooms() {
  std::cout << "rooms:" << std::endl;
  for (auto pair : rooms) {
    print_pair(std::make_pair(pair.first, rooms[pair.first].entity));
  };
};

void LevelBuilder::print_hallways() {
  std::cout << "hallways:" << std::endl;
  for (auto pair : hallways) {
    print_pair(std::make_pair(pair.first, hallways[pair.first].entity));
  };
};