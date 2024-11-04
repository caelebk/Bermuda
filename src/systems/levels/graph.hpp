#include <vector>

#include "environment.hpp"

#define MIN_CONNECTIONS 1
#define MAX_CONNECTIONS 4

#define DOOR_SCALAR 1

using Graph = std::unordered_map<int, std::set<int>>;
using DirectedEdges = std::unordered_map<int, Direction>;
using DirectedGraph = std::unordered_map<int, std::unordered_map<int, Direction>>;

// These two may not belong here.
Direction get_opposite_direction(Direction direction);
std::vector<int> get_random_door_positions(int num_rooms, int min, int max);

int count_edges_with_direction(Direction direction, DirectedEdges& directed_adjacencies);
std::vector<int> get_connecting_vertices_with_direction(Direction direction, DirectedEdges& directed_adjacencies);

DirectedGraph expand_graph_to_random_directed_graph(Graph& adjacency_list);
Graph generate_random_graph(std::vector<int>& rooms, std::vector<int>& adjacency_list);