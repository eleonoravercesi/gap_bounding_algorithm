/*
*Just a library of functions for reading files
 */

#include "../include/readers.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <numeric> // Required for std::lcm


using namespace std;

vector<VertexFraction> read_ancestors(const string& filename) {
    vector<VertexFraction> ancestors;
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
            stringstream val_ss(values_str);
            string token;

            vector<int> numerators;
            vector<int> denominators;

            int den = 1; // Default denominator just in case
            while (val_ss >> token) {
                size_t slash_pos = token.find('/');
                if (slash_pos != string::npos) {
                    double num = stod(token.substr(0, slash_pos));
                    double den = stod(token.substr(slash_pos + 1));
                    numerators.push_back((int) num);
                    denominators.push_back((int) den);
                } else {
                    numerators.push_back(0);
                    denominators.push_back(1);
                }
            }

            // Consistent check: number of edges m = n*(n-1)/2
            int m = n * (n - 1) / 2;
            assert(m == numerators.size());
            assert(m == denominators.size());


            // Now: lcm
            int overall_lcm = denominators[0];
            for (size_t i = 1; i < denominators.size(); ++i) {
                overall_lcm = std::lcm(overall_lcm, denominators[i]);
            }

            for (size_t i = 0; i < numerators.size(); ++i) {
                numerators[i] = numerators[i] * overall_lcm / denominators[i];
            }

            VertexFraction vertex_fraction;
            vertex_fraction.den = overall_lcm;
            vertex_fraction.v = numerators;

            // push the constructed VertexFraction into the result vector
            ancestors.push_back(vertex_fraction);
        }
    }
    return ancestors;
}

vector<vector<int>> read_upper_triangle_graphs(const string& filename) {
    vector<vector<int>> graphs;
    ifstream file(filename);

    // throw an error if the file does not exist
    if (!file.is_open()) {
        cerr << "Error: Cannot open or create file " << filename << "\n";
        exit(1);
    }

    string line;
    while (getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // First line of each graph: number of vertices
        int n = stoi(line);

        vector<int> g;

        // Read the upper triangle
        for (int i = 0; i < n - 1; ++i) {  // n-1 rows (last row is empty)
            getline(file, line);
            if (line.empty()) continue;

            istringstream iss(line);
            int weight;
            int j = i + 1;  // Start from diagonal + 1

            while (iss >> weight) {
                g.push_back(weight);
                j++;
            }
        }

        graphs.push_back(g);
    }

    file.close();
    return graphs;
}