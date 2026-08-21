/* *
 * Created using Gurobot
 * */

#ifndef SOLVERS_H
#define SOLVERS_H

#include <vector>
#include <utility>  // for std::pair
#include <map>

using std::vector;
using std::pair;
using std::map;

/**
 * @brief Stores the result of a Traveling Salesman Problem (TSP) computation.
 *
 * The structure contains the edges of the resulting tour, its total cost,
 * and an indicator of whether a feasible solution was successfully found.
 */
struct TSPSolution {
    std::vector<std::pair<int, int>> tour_edges;  // List of edges (i,j) in the tour
    double tour_value;                             // Total cost of the tour
    bool success;                                  // Whether a solution was found

    // Constructor for easy initialization
    TSPSolution() : tour_value(0.0), success(false) {}
};

/**
 * @brief Stores the result of a Graph-TSP computation.
 *
 * The solution is represented as a walk, where each edge is associated with
 * its multiplicity. The structure also stores the multiplicity of each node,
 * the total cost of the walk, and an indicator of whether a feasible solution
 * was successfully found.
 */
struct GraphTSPSolution {
    vector<pair<pair<int, int>, int>> walk_edges;
    vector<int> node_multiplicities;
    double tour_value;
    bool success;
};

#endif // SOLVERS_H
