#include <random>
#include <iostream>

#include "random.hpp"
#include "graph.hpp"
#include <numeric>

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

std::vector<int> get_random_door_positions(std::vector<int>& connected_rooms, int min, int max) {
  std::vector<int> positions;

  // Count the number of elements with the specified direction.
  int count = connected_rooms.size();

  // Generate unique positions with a minimum distance of DOOR_SCALAR between them.
  while ((int)positions.size() < count) {
    int  position      = getRandInt(min, max);
    bool is_valid = true;

    // Ensure the position is at least DOOR_SCALAR units away from both min and max.
    if (position <= min + DOOR_SCALAR || position >= max - DOOR_SCALAR) {
      is_valid = false;
    }

    // Ensure each new position is at least DOOR_SCALAR units away from existing ones.
    for (int existing_pos : positions) {
      if (std::abs(existing_pos - position) < DOOR_SCALAR) {
        is_valid = false;
        break;
      }
    }

    if (is_valid) {
      positions.push_back(position);
    } else {
      // Try again from scratch.
      positions = std::vector<int>();
    }
  }

  std::sort(positions.begin(), positions.end());
  return positions;
}

DirectedGraph expand_graph_to_random_directed_graph(Graph& graph) {
    // For each edge in the adjacency list, associate each to one of four randomized directions to make our doors make physical sense when rendered.
    // Expand the adjacency list into a directed adjacency list that stores this assigned direction as a nested map.
    DirectedGraph directed_graph;
    std::set<std::pair<int, int>> processed_pairs;

    std::vector<Direction> directions = {Direction::NORTH, Direction::EAST, Direction::SOUTH, Direction::WEST};
    for (const auto& pair : graph) {
        int room_number = pair.first;
        std::vector<int> edges(pair.second.begin(), pair.second.end());

        for (int i = 0; i < (int) edges.size(); i++) {
            int other_room = edges[i];

            Direction random_direction;
            do {
                random_direction = directions[getRandInt(0, directions.size() - 1)];
            } while (
                // Keep selecting another direction until:
                // 1. That direction has not already been added to this room, up to thrice (since it may be impossible to fit doors onto one wall if there are enough of them).
                // TODO: Remove magic number.
                count_edges_with_direction(random_direction, directed_graph[room_number]) >= 3
            );

            // If we've already processed this pair, move on.
            if (processed_pairs.count({room_number, other_room}) || processed_pairs.count({other_room, room_number})) {
                continue;
            }

            directed_graph[room_number][other_room] = random_direction;

            // Set opposite direction for the other room.
            Direction opposite_direction = get_opposite_direction(random_direction);
            directed_graph[other_room][room_number] = opposite_direction;

            // Mark this pair as processed.
            processed_pairs.insert({room_number, other_room});
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

Graph generate_random_graph(std::vector<int>& rooms, std::vector<int>& densities) {
    std::random_device rd;
    std::default_random_engine rng(rd()); 
    // Make a non-ECS adjacency list as our workspace. This will be ECS-ified once we finish the graph.
    Graph graph;
        
    int total_rooms = 0;
    for (int n : rooms) {
        total_rooms += n;
    }

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
            graph[room_number].insert(random_path[i+1]);
        } else if (i == rooms_in_cluster - 1) {
            graph[room_number].insert(random_path[i-1]);
        } else {
            graph[room_number].insert(random_path[i+1]);
            graph[room_number].insert(random_path[i-1]);
        }
        }

        for (int i = 0; i < rooms_in_cluster; i++) {
        int room_number = random_path[i];
        // Factor in the density probability and check that the room isn't the last one or already has the maximum number of connections.
        if (getRandInt(1, 100) < p || room_number == random_path.back() || graph[room_number].size() >= MAX_CONNECTIONS) {
            continue;
        }

        // Connect this room to up to at most four other rooms in total.
        int num_connections = getRandInt(0, MAX_CONNECTIONS - graph[room_number].size());
        while (num_connections > 0) {
            int other_room_number;
            do {
            other_room_number = random_path[getRandInt(0, rooms_in_cluster - 1)];
            // Keep selecting another room in this cluster until it fulfills all of:
            // 1. It is not equal to the connectee room.
            // 2. It is not already connected to the connectee room.
            // 3. It does not yet have the maximum number of connections.
            } while (
            other_room_number == room_number
            || graph[room_number].count(other_room_number) != 0
            || graph[other_room_number].size() >= MAX_CONNECTIONS
            );

            // Leave the exit room as a funnel point so we can place a mini-boss there. Also leave at least one connection to delineate map difficulty transitions.
            if (
            other_room_number == random_path.back()
            || (other_room_number == first_room_number && graph[first_room_number].size() >= MAX_CONNECTIONS - 1)
            ) {
            num_connections--;
            continue;
            }

            // Add the connection.
            graph[room_number].insert(other_room_number);
            graph[other_room_number].insert(room_number);

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
        graph[transition_room].insert(transition_room+1);
        graph[transition_room+1].insert(transition_room);
        transition_room += rooms[i];
    }

    // Debug output.
    std::cout << "Graph:" << std::endl;
    for (int i = 0; i < total_rooms; i++) {
        std::cout << "room " << i << ": ";
        for (int j : graph[i]) {
        std::cout << j << " ";
        }
        std::cout << "\n";
    }

    return graph;
};