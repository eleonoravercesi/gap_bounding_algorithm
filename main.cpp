/**
 * This file containts several routine; you can run whatever you wish just selecting the right flag; more specifically
 * "opt_plus" run the opt_plus algorithm on vertices of a given size and produces a file with the maximum gap
 **/

#include <filesystem>
#include <format>
#include "ppl.hh"
#include <iostream>
#include "include/readers.h"
#include <vector>
#include "utils.h"
#include "include/gap_related_functions.h"
#include <string>
#include <fstream>
#include <chrono>

#include "solvers.h"


using namespace std;
using std::chrono::duration;

void opt_plus_on_n(string n) {
    /** Run opt_plus on the vertices of size n and write the results in a csv file with idx,opt_plus,gap_plus,runtime
     * idx is the index of the vertex in the list of vertices
     * opt_plus is the value of opt_plus for that vertex
     * gap_plus is 1/opt_plus
     * runtime is the time taken to compute opt_plus for that vertex
     */
    string filename = format("/home/vercee/Documents/math_prog_extended/vertices/vertices{}.txt", n);
    int n_int = stoi(n);
    vector<vector<double>> x_list = read_vertices_chm(filename);

    // Open a fine to write idx,opt_plus,gap_plus,runtime
    ofstream fout;
    fout.open(format("/home/vercee/Documents/math_prog_extended/output/results_gap_plus_{}.csv", n));

    // Write the header
    fout << "idx,opt_plus,gap_plus,runtime\n";

    // Set a high precision so we can recover fractions
    fout << setprecision(20);

    // For loop on the vectors
    vector<double> gaps;
    for (size_t i = 0; i < x_list.size(); ++i) {
        vector<double> x_0 = x_list[i];

        // Start opt solution
        auto t1 = std::chrono::high_resolution_clock::now();
        double opt_plus_val = opt_plus(x_0, n_int);
        auto t2 = std::chrono::high_resolution_clock::now();

        /* Getting number of milliseconds as a double. */
        duration<double, std::milli> ms_double = t2 - t1;

        gaps.push_back(opt_plus_val);
        cout << "\t \t \t Gap for vertex " << i << ": " << 1 / opt_plus_val << endl;
        cout << "-----------------------------------" << endl;

        // Now, write on file
        fout << i << "," << opt_plus_val << "," << 1 / opt_plus_val << "," << ms_double << "\n";
    }
    fout.close();
}

void test_scip_tsp(string n) {
    int n_int = stoi(n);
    std::vector<double> costs = {
        0.192308,0.153846,0.038462,0.076923,0.115385,0.230769,0.307692,0.192308,0.230769,0.269231,0.307692,0.269231,
        0.115385,0.192308,0.230769,0.192308,0.076923,0.307692,0.038462,0.076923,0.192308,0.269231,0.038462,0.153846,
        0.230769,0.192308,0.192308,0.230769
    };

    auto out = solve_tsp_scip(costs, n_int, 0);
    cout << get<0>(out) << endl;

    vector<pair<int, int>> tour = get<1>(out);
    for (int i = 0; i < tour.size(); ++i) {
        cout << tour[i].first << " and " << tour[i].second << endl << " ";
    }
    cout << endl;

}

void test_connected_components() {
    vector<vector<int>> adj = {
        {0,3}, {0, 4}, {1, 3}, {1, 7}, {2, 6}, {2, 8}, {4, 5},
        {5, 7}, {6, 8}
    };

    vector<int> S = oneComponent(adj);

    for (size_t i = 0; i < S.size(); ++i) {
        cout << S[i] << " ";
    }
    cout << endl;


}

int main(int argc, char *argv[]){
    // string what_to_run = argv[1];
    // string n = argv[2];

    // string what_to_run = "tsp_example";
    string what_to_run = "opt_plus";
    //string what_to_run = "connected_components";
    string n = "11";


    if (what_to_run == "opt_plus") {
        opt_plus_on_n(n);
    }
    else if (what_to_run == "tsp_example") {
        test_scip_tsp(n);
    }
    else if (what_to_run == "connected_components") {
        test_connected_components();
    }
}