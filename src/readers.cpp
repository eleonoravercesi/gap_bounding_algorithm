/*
*Just a library of functions for reading files
 */

#include "../include/readers.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>

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

vector<pair<int, vector<double>>> read_ancestors(const string& filename) {
    vector<pair<int, vector<double>>> ancestors;
    ifstream vertices_file(filename);
    string line;

    // Skip the header line: "n,xi"
    if (!getline(vertices_file, line)) return ancestors;

    while (getline(vertices_file, line)) {
        if (line.empty()) continue;

        // Use stringstream to split by comma
        stringstream ss(line);
        string id_str, values_str;

        // Get everything before the comma (ID) and after (the values)
        if (getline(ss, id_str, ',') && getline(ss, values_str)) {
            int n = stoi(id_str);
            vector<double> vertex;
            stringstream val_ss(values_str);
            string token;

            while (val_ss >> token) {
                size_t slash_pos = token.find('/');
                if (slash_pos != string::npos) {
                    double num = stod(token.substr(0, slash_pos));
                    double den = stod(token.substr(slash_pos + 1));
                    vertex.push_back(num / den);
                } else {
                    vertex.push_back(stod(token));
                }
            }

            // Consistent check: number of edges m = n*(n-1)/2
            int m = n * (n - 1) / 2;
            assert(m == vertex.size());

            ancestors.push_back({n, vertex});
        }
    }
    return ancestors;
}