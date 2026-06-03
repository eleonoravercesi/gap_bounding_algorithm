#ifndef READERS_H
#define READERS_H

#include <vector>
#include <string>
using namespace std;

std::vector<std::vector<double>> read_vertices_chm(const std::string& filename);

vector<pair<int, vector<double>>> read_ancestors(const string& filename);


#endif
