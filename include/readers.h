/*
 *Just a library of functions for reading files
 */
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Function for reading the vertices files as produced by Cook, William; Hougardy, Stefan; Petrich, Moritz, 2026, "Vertices of the Subtour Polytope
// See here https://bonndata.uni-bonn.de/dataset.xhtml?persistentId=doi:10.60507/FK2/JK95PC
inline vector<vector<double>> read_vertices_chm(const string& filename) {
    vector<vector<double>> vertices;

    // Open the file
    ifstream vertices_file(filename.c_str());

    string line;

    // Use a while loop together with the getline() function to read the file line by line
    while (getline (vertices_file, line)) {
        // Skip all the lines that begins with c
        vector<double> vertex;
        if (not line.starts_with('c')) {
            // Split the line into tokens using space as a delimiter
            size_t pos = 0;
            string token;
            while ((pos = line.find(' ')) != string::npos) {
                token = line.substr(0, pos);
                line = line.substr(pos + 1);

                // Now that you have the token, split it acording to "/"
                string num;
                string den;
                num = line.substr(0, line.find('/'));
                den = line.substr(2, line.find('/'));

                // Convert num and den to float and push back the value num/den to the vertex vector
                double num_float = stof(num);
                double den_float = stof(den);

                vertex.push_back(num_float / den_float);
            }
        }
        vertices.push_back(vertex);
    }

    return vertices;
}