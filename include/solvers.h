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

/**
 * Solves the Traveling Salesman Problem
 *
 * @param n Number of nodes
 * @param C Vector of costs for upper triangle (size = n*(n-1)/2)
 *          Order: (0,1), (0,2), ..., (0,n-1), (1,2), (1,3), ..., (n-2,n-1)
 * @param verbosity Verbosity level:
 *          0 --> Completely silent
 *          1 --> Only custom printing
 *          2 --> Everything
 * @return TSPSolution containing tour edges and total cost
 */
//TODO ma dove lo uso???
TSPSolution solve_tsp(int n, const std::vector<double>& C, int verbosity);


/**
 * Solves the Graph Traveling Salesman Problem
 *
 * @param n Number of nodes
 * @param C Vector of costs for upper triangle (size = n*(n-1)/2)
 *          Order: (0,1), (0,2), ..., (0,n-1), (1,2), (1,3), ..., (n-2,n-1)
 * @param verbosity Verbosity level:
 *          0 = silent
 *          1 = basic output, custom printing
 *          2 = full Gurobi output
 * @return GraphTSPSolution containing walk edges with multiplicities and total cost
 */
GraphTSPSolution solve_graph_tsp(int n, const map<pair<int,int>, double>& edge_costs, int verbosity);
#endif // SOLVERS_H
