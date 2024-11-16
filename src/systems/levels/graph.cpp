#include <random>
#include <iostream>
#include <cmath>

#include <numeric>

#include "graph.hpp"
#include "level_builder.hpp"
#include "level_factories.hpp"
#include "random.hpp"

Direction get_opposite_direction(Direction direction) {
    Direction opposite;
    switch(direction) {
        case Direction::NORTH:
        opposite = Direction::SOUTH;
        break;
        case Direction::EAST:
        opposite = Direction::WEST;
        break;
        case Direction::SOUTH:
        opposite = Direction::NORTH;
        break;
        case Direction::WEST:
        opposite = Direction::EAST;
        break;
    }
    return opposite;
};

int count_edges_with_direction(Direction direction, DirectedEdges& directed_edges) {
    int count = 0;
    for (const auto& pair : directed_edges) {
        if (pair.second == direction) {
            count++;
        }
    }
    return count;
}

std::vector<int> get_connecting_vertices_with_direction(Direction direction, DirectedEdges& directed_edges) {
    std::vector<int> connected_rooms;

    for (const auto& pair : directed_edges) {
        int       other_room     = pair.first;
        Direction pair_direction = pair.second;
        if (direction == pair_direction) {
            connected_rooms.push_back(other_room);
        }
    }

    return connected_rooms;
}

std::vector<int> get_random_door_positions(int num_doors, int min, int max) {
  std::vector<int> positions;
  bool is_valid;

  while ((int)positions.size() < num_doors) {
    int  position      = getRandInt(min, max);
    is_valid = true;

    // Ensure the position is at least DOOR_SIZE units away from both min and max.
    if (position <= min + DOOR_SIZE || position >= max - DOOR_SIZE) {
      is_valid = false;
    }

    // Ensure each new position is at least DOOR_SIZE units away from existing ones.
    for (int existing_pos : positions) {
      if (std::abs(existing_pos - position) < DOOR_SIZE + 1) {
        is_valid = false;
        break;
      }
    }

    if (is_valid) {
      positions.push_back(position);
    } else {
      // Try again from scratch.
      while (positions.size() > 0) {
        positions.pop_back();
      }
    }
  }

  std::sort(positions.begin(), positions.end());
  return positions;
}

DirectedGraph expand_graph_to_random_directed_graph(Graph& graph) {
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::vector<Direction> directions = {Direction::NORTH, Direction::EAST, Direction::SOUTH, Direction::WEST};

    // For each edge in the adjacency list, associate each to one of four randomized directions.
    // Also, expand the adjacency list into a directed adjacency list that stores this assigned direction as a nested map.
    DirectedGraph directed_graph;
    std::set<std::pair<int, int>> processed_pairs;

    for (const auto& pair : graph) {
        int room_number = pair.first;
        std::vector<int> edges(pair.second.begin(), pair.second.end());

        for (int i = 0; i < (int) edges.size(); i++) {
            int other_room = edges[i];

            // If we've already processed this pair, move on.
            if (processed_pairs.count({other_room, room_number}) > 0) {
                continue;
            }

            Direction random_direction;
            do {
                std::shuffle(directions.begin() + 1, directions.end() - 1, rng);
                random_direction = directions[getRandInt(0, directions.size() - 1)];
            } while (
                // Keep selecting another direction until that direction has not already been added to this room, up to a predefined number,
                // just so we don't get every wall on the same door.
                count_edges_with_direction(random_direction, directed_graph[room_number]) >= MAX_DOORS_PER_WALL
            );

            directed_graph[room_number][other_room] = random_direction;

            // Set opposite direction for the other room.
            Direction opposite_direction = get_opposite_direction(random_direction);
            directed_graph[other_room][room_number] = opposite_direction;

            // Mark this pair as processed.
            processed_pairs.insert({room_number, other_room});
            processed_pairs.insert({other_room, room_number});
        }
    }

    // Debugging output.
    std::cout << "Adjacency List with Door Connections" << std::endl;
    for (int room_number = 0; room_number < (int) directed_graph.size(); room_number++) {
        DirectedEdges& directions = directed_graph[room_number];
        std::cout << "room " << room_number << ": ";
        for (auto const& direction_pair : directions) {
        int other_room = direction_pair.first;
        Direction direction = direction_pair.second;
        switch(direction) {
            case Direction::NORTH: std::cout << "(NORTH," << other_room << ") "; break;
            case Direction::EAST: std::cout << "(EAST," << other_room << ") "; break;
            case Direction::SOUTH: std::cout << "(SOUTH," << other_room << ") "; break;
            case Direction::WEST: std::cout << "(WEST," << other_room << ") "; break;
        }
        }
        std::cout << "\n";
    }

    return directed_graph;
}

Graph generate_random_graph() {
    std::random_device rd;
    std::default_random_engine rng(rd()); 

    // Make a non-ECS adjacency list as our workspace. This will be ECS-ified once we finish the graph.
    Graph graph;

    // Assume the tutorial rooms are placed before any actual gameplay rooms.
    int first_room_in_cluster = 0; 
    uint normal_rooms;
    for (int cluster = 0; cluster < (int) ROOM_CLUSTERS.size(); cluster++) {
        int rooms_in_cluster = ROOM_CLUSTERS[cluster];
        normal_rooms = rooms_in_cluster - 1;
        unsigned int max_connections = normal_rooms - 1;

        // If there is only one room in this cluster for whatever reason, just connect it and continue.
        if (rooms_in_cluster == 1) {
            // Mark the first room that begins a new difficulty area.
            first_room_in_cluster += ROOM_CLUSTERS[cluster];

            graph[first_room_in_cluster];
            continue;
        }

        // Generate a random path through this cluster of rooms, since we want our level to be fully-connected.

        std::vector<int> random_path(rooms_in_cluster);
        for (int i = 0; i < rooms_in_cluster; ++i) {
            random_path[i] = first_room_in_cluster + i;
        }
        // Keep the first and last nodes fixed, so we can connect difficulty areas via first_room_in_cluster and the elements of the input vector.
        std::shuffle(random_path.begin() + 1, random_path.end() - 1, rng);

        // Fill in the adjacency list for the path we just generated.
        for (int i = 0; i < rooms_in_cluster; i++) {
            int room_number = random_path[i];
            if (i == 0) {
                graph[room_number].insert(random_path[i+1]);
            } else if (i == rooms_in_cluster - 1) {
                graph[room_number].insert(random_path[i-1]);
            } else {
                graph[room_number].insert(random_path[i+1]);
                graph[room_number].insert(random_path[i-1]);
            }
        }

        // Notice that we skip the boss room because we want to keep it a funnel point that delineates area transitions.
        for (unsigned int i = 0; i < normal_rooms; i++) {
            int room_number = random_path[i];

            // Naturally, skip any rooms that are already too full.
            if (graph[room_number].size() >= max_connections) {
                continue;
            }

            // Randomly connect this room to other ones.
            // Again, since the boss room should be sparse, decrement max_connections by one.
            int num_connections = getRandInt(0, max_connections - graph[room_number].size());

            while (num_connections > 0) {
                int other_room_number;

                do {
                    other_room_number = random_path[getRandInt(0, rooms_in_cluster - 1)];
                } while (
                    // Discard this candidate other room if any of these are true:
                    // 1. It is equal to the connectee room.
                    // 2. It is already connected to the connectee room.
                    // 3. It already has the maximum number of connections.
                    other_room_number == room_number
                    || graph[other_room_number].count(room_number) > 0
                    || graph[other_room_number].size() >= max_connections
                );

                // Add the connection.
                graph[room_number].insert(other_room_number);
                graph[other_room_number].insert(room_number);

                // decrement num_connections
                num_connections--;
            }
        }

        // Mark the first room that begins a new difficulty area.
        first_room_in_cluster += ROOM_CLUSTERS[cluster];
    }

    // Connect all the boss rooms together that delineate a new difficulty area. The boss rooms are guaranteed to have available connections.
    int boss_room;
    for (int i = 0; i < (int) MINIBOSS_ROOMS.size() - 1; i++) {
        boss_room = MINIBOSS_ROOMS[i];
        graph[boss_room].insert(boss_room + 1);
        graph[boss_room + 1].insert(boss_room);
    }

    // Debug output.
    std::cout << "Graph:" << std::endl;
    int total_rooms = 0;
    for (int n : ROOM_CLUSTERS) {
        total_rooms += n;
    }
    for (int i = 0; i < total_rooms; i++) {
        std::cout << "room " << i << ": ";
        for (int j : graph[i]) {
        std::cout << j << " ";
        }
        std::cout << "\n";
    }

    return graph;
};
