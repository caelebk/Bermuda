#include <numeric>
#include <algorithm>
#include <random>

#include "level.hpp"

#include "common.hpp"
#include "room.hpp"

#define MIN_CONNECTIONS 1
#define MAX_CONNECTIONS 4

// An enumeration for programatically defining directions.
enum Direction {
  NORTH = 0,
  EAST = 1,
  SOUTH = 2,
  WEST = 3
};

int get_random(int x, int y) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribution(x, y);
    return distribution(gen);
}

LevelBuilder::LevelBuilder(){};

RoomBuilder& LevelBuilder::room(std::string s_id) {
  return rooms[s_id];
};

HallwayBuilder& LevelBuilder::hallway(std::string s_id) {
  return hallways[s_id];
};

void LevelBuilder::connect(Entity& connectee, Entity& connector) {
  registry.adjacencies.get(connectee).neighbours.push_back(connector);
};

LevelBuilder& LevelBuilder::connect_room_to_hallway(std::string r_id,
                                                    std::string d1_id,
                                                    std::string h_id,
                                                    std::string d2_id) {
  RoomBuilder&    room    = rooms[r_id];
  HallwayBuilder& hallway = hallways[h_id];

  connect(room.entity, hallway.entity);
  connect(hallway.doors[d1_id], room.doors[d2_id]);
  connect(hallway.entity, room.entity);
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

void LevelBuilder::generate_level_from_graph(std::unordered_map<int, std::set<int>>& adjacency_list) {
  std::unordered_map<int, std::unordered_map<int, Direction>> room_directions;
  std::set<std::pair<int, int>> processed_pairs;

  std::random_device rd;
  std::default_random_engine rng(rd());

  std::vector<Direction> directions = {NORTH, EAST, SOUTH, WEST};
  for (const auto& pair : adjacency_list) {
    int room_number = pair.first;
    std::vector<int> edges(pair.second.begin(), pair.second.end());
    std::shuffle(directions.begin(), directions.end(), rng);

    for (int i = 0; i < (int) edges.size(); i++) {
      int other_room = edges[i];

      // Skip if already processed in reverse
      if (processed_pairs.count({room_number, other_room}) || processed_pairs.count({other_room, room_number})) {
        continue;
      }

      room_directions[room_number][other_room] = directions[i];

      // Set opposite direction for the other room
      Direction opposite_direction;
      switch(directions[i]) {
        case NORTH: opposite_direction = SOUTH; break;
        case EAST: opposite_direction = WEST; break;
        case SOUTH: opposite_direction = NORTH; break;
        case WEST: opposite_direction = EAST; break;
      }
      room_directions[other_room][room_number] = opposite_direction;

      // Mark this pair as processed
      processed_pairs.insert({room_number, other_room});
    }
  }

  // Debugging output
  std::cout << "directions output" << std::endl;
  for (int room_number = 0; room_number < (int) room_directions.size(); room_number++) {
    std::unordered_map<int, Direction>& directions = room_directions[room_number];
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
  std::cout << "final output" << std::endl;
  for (int i = 0; i < total_rooms; i++) {
    std::cout << "room " << i << ": ";
    for (int j : adjacency_list[i]) {
      std::cout << j << " ";
    }
    std::cout << "\n";
  }

  return adjacency_list;
};

// Note since we transition when moving spaces, these graphs don't have to be planar.
void LevelBuilder::generate_random_level(std::vector<int> rooms, std::vector<int> densities) {
  std::unordered_map<int, std::set<int>> random_graph = generate_random_graph(rooms, densities);
  generate_level_from_graph(random_graph);
};

void LevelBuilder::buildRoomOne() {
  room(ROOM_ONE).up(Y_10U).right(X_14U).down(Y_5U).right(X_6U).down(Y_5U).left(
      X_20U);
};

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
};

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