/*
*Just a library of functions for reading files
 */

#include "../include/readers.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

// Function for reading the vertices files as produced by Cook, William; Hougardy, Stefan; Petrich, Moritz, 2026, "Vertices of the Subtour Polytope
// See here https://bonndata.uni-bonn.de/dataset.xhtml?persistentId=doi:10.60507/FK2/JK95PC
vector<vector<double>> read_vertices_chm(const string& filename) {
    vector<vector<double>> vertices;
    ifstream vertices_file(filename);
    string line;

    while (getline(vertices_file, line)) {
        if (line.empty() || line[0] == 'c') continue;

        vector<double> vertex;
        stringstream ss(line);
        string token;

        // This correctly splits the line by spaces
        while (ss >> token) {
            size_t slash_pos = token.find('/');
            if (slash_pos != string::npos) {
                // Split the token (e.g., "1/2") into parts
                string num_str = token.substr(0, slash_pos);
                string den_str = token.substr(slash_pos + 1);

                double num = stod(num_str);
                double den = stod(den_str);

                vertex.push_back(num / den);
            } else {
                // Handle cases without a slash if necessary
                vertex.push_back(stod(token));
            }
        }
        if (!vertex.empty()) {
            vertices.push_back(vertex);
        }
    }
    return vertices;
}