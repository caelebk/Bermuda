#include <vector>

#include "environment.hpp"

#define DOOR_SIZE 1
#define MAX_DOORS_PER_WALL 2

using Graph = std::unordered_map<int, std::set<int>>;
using DirectedEdges = std::unordered_map<int, Direction>;
using DirectedGraph = std::unordered_map<int, DirectedEdges>;

Direction get_opposite_direction(Direction direction);
std::vector<int> get_random_door_positions(int num_rooms, int min, int max);

int count_edges_with_direction(Direction direction, DirectedEdges& directed_adjacencies);
std::vector<int> get_connecting_vertices_with_direction(Direction direction, DirectedEdges& directed_adjacencies);

DirectedGraph expand_graph_to_random_directed_graph(Graph& adjacency_list);
Graph generate_random_graph();