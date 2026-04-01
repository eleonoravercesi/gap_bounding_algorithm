#include <filesystem>

#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include "ppl.hh"
#include <iostream>
#include "include/readers.h"
#include <vector>
extern "C" {
    #include "concorde.h"
}
#include "include/solvers.h"
#include <tuple>
#include "include/gap_related_functions.h"

int main(int argc, char *argv[]){

    // vector<vector<float>> x_list = read_vertices_chm(argv[1]);
    // vector<float> x_0 = x_list[0];
    // double opt_plus_val = opt_plus(x_0, stoi(argv[2]));
    // cout << "opt_plus_val = " << opt_plus_val << endl;

    string filename = "/home/vercee/Documents/math_prog_extended/vertices/vertices6.txt";
    string n = "6";
    vector<vector<double>> x_list = read_vertices_chm(filename.c_str());
    vector<double> x_0 = x_list[0];
    double opt_plus_val = opt_plus(x_0, stoi(n));
    cout << "opt_plus_val = " << opt_plus_val << endl;

}