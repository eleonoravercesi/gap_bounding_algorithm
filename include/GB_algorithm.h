#ifndef OPTII_H
#define OPTII_H

#include <vector>
#include <map>
#include <utility>
#include "gurobi_c++.h"

// Type definition for a Walk as defined in the source
using Walk = std::map<std::pair<int, int>, int>;

// Edge type
using Edge = pair<int, int>;
using Vertex = map<pair<int, int>, double>;

// GB results structure
struct GBAlgorithmResult {
    double gapII;                               // gapII of x0
    double family_gapII;                        // family-gapII of x0
    vector<pair<Walk, double>> opt_variables;   // Optimal walks with dual values
    Edge worst_one_edge;                        // One-edge with highest factor
    bool has_worst_edge;
    int iterations;                             // Number pof iterations, for the GBe
    bool success;                               // Solution found?
};


// Result structure
struct OptIISolution {
    double opt_value;
    std::vector<std::pair<Walk, double>> opt_variables;
    std::map<std::pair<int, int>, double> c_star;
    double solve_time;
    bool success;
    int iterations;
};

struct GraphTSPSolution;

/**
 * Solves the OPT-II problem using a cutting plane approach.
 * * @param n Number of nodes
 * @param x Current fractional solution (edge weights)
 * @param starting_walks Initial set of walks for constraints
 * @param tol Tolerance for the cutting plane termination
 * @param verbosity Output verbosity level (0-2)
 * @return OptIISolution containing the results
 */
OptIISolution solve_optII(
    int n,
    const std::map<std::pair<int, int>, double>& x,
    const std::vector<Walk>& starting_walks,
    double tol,
    int verbosity
);

GBAlgorithmResult GB_algorithm(
    int n,
    const map<pair<int,int>, double>& x0,
    const vector<Walk>& starting_walks = {},
    double tol = 1e-6,
    int verbosity = 1
);

GBAlgorithmResult GBe_algorithm(
    int n,
    const Vertex& x0,
    double target_gapII,
    int max_iterations,
    double tol,
    int verbosity
);

#endif