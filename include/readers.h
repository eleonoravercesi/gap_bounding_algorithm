#ifndef READERS_H
#define READERS_H

#include <vector>
#include <string>
#include "vertices_generator.h"
using namespace std;


std::vector<std::vector<double>> read_vertices_chm(const std::string& filename);

vector<VertexFraction> read_ancestors(const string& filename);

vector<vector<int>> read_upper_triangle_graphs(const string& filename);

#endif // READERS_H
