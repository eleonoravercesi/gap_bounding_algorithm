#ifndef READERS_H
#define READERS_H

#include <vector>
#include <string>
#include "vertices_generator.h"
using namespace std;


/**
 * Read ancestor file in format n, components of x as fractions
 *
 * @param filename the filename of the ancestors having n, x_0 x_1 ... x_m at every line
 * @return a vector of vertices
 */
vector<VertexFraction> read_ancestors(const string& filename);

/**
 * Read the graphs in upper triangle format as produces by showg of nauty, that is:
    6
    0 0 1 1 1
    0 1 1 1
    1 1 1
    0 0
    0
 * @param filename : the name of the file
 * @return a vector<vector<int>> with the matrices.
 */
vector<vector<int>> read_upper_triangle_graphs(const string& filename);

#endif // READERS_H
