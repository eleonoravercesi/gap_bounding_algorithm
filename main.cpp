#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include "ppl.hh"
#include <iostream>
#include "include/readers.h"
#include <vector>

int main(int argc, char* argv[]) {
    vector<vector<float>> vertices = read_vertices_chm(argv[1]);

    for (vector<float> x: vertices) {
        for (float x_e : x) {
            cout << x_e << " ";
        }
        cout << endl;
    }
}
