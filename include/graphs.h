#ifndef GRAPHS_H
#define GRAPHS_H
#include <vector>
#include "vertices_generator.h"

using namespace std;

// Compute canonical form of a nauty graph of size n
//graph* get_canonical_form(graph* g, int nd, int original_n);

// Check if two nauty graphs of size n are isomorphic
//bool are_isomorphic_graphs(graph* g1, graph* g2, int n);
bool are_isomorphic_vertices(VertexFraction v1, VertexFraction v2, int n, int verbose = 1);

// Filter graphs, returning indices of non-isomorphic representatives
vector<int> filter_isomorphic_graphs(vector<VertexFraction> graphs, int n, int verbose = 1);

// Free all graphs in a vector
//void free_graph_vector(vector<graph*>& graphs, int n);

#endif // GRAPHS_H