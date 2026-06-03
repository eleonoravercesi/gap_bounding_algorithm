/* *
 * Created using Gurobot
 * */

#ifndef SOLVERS_H
#define SOLVERS_H

#include <vector>
#include <utility>  // for std::pair

using namespace std;
#include <map>

// Structure to hold TSP solution results
struct TSPSolution {
    std::vector<std::pair<int, int>> tour_edges;  // List of edges (i,j) in the tour
    double tour_value;                             // Total cost of the tour
    bool success;                                  // Whether a solution was found

    // Constructor for easy initialization
    TSPSolution() : tour_value(0.0), success(false) {}
};

// struct to hold the Graphic TSP solution
struct GraphTSPSolution {
    vector<pair<pair<int, int>, int>> walk_edges;
    vector<int> node_multiplicities;
    double tour_value;
    bool success;
};

// TSP Solver Function
// Inputs:
//   n: Number of nodes in the TSP
//   C: Upper triangle cost vector of size n*(n-1)/2
//      Ordered as: (0,1), (0,2), ..., (0,n-1), (1,2), (1,3), ..., (n-2,n-1)
// Returns:
//   TSPSolution struct containing:
//     - tour_edges: List of edges in optimal tour
//     - tour_value: Total cost of the tour
//     - success: true if solution found, false otherwise
TSPSolution solve_tsp(int n, const std::vector<double>& C, int verbosity);



GraphTSPSolution solve_graph_tsp(int n, const map<pair<int,int>, double>& edge_costs, int verbosity);
#endif // SOLVERS_H
