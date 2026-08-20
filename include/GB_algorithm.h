#ifndef OPTII_H
#define OPTII_H

#include <vector>
#include <map>
#include <utility>
#include <utils.h>

/**
 * @brief Stores the output of the Gap-Bounding (GB) algorithm.
 *
 * This structure contains the gap values computed for the input solution,
 * the optimal walks and their corresponding dual values, and information
 * about the worst edge identified by the algorithm.
 */
struct GBAlgorithmResult {
    double gapII;                               // gapII of x0
    double family_gapII;                        // family-gapII of x0
    vector<pair<Walk, double>> opt_variables;   // Optimal walks with dual values
    Edge worst_one_edge;                        // One-edge with highest factor
    bool has_worst_edge;
    int iterations;                             // Number pof iterations, for the GBe
    bool success;                               // Solution found?
};


/**
 * @brief Stores the result of the OPT-II optimization procedure.
 *
 * This structure contains the optimal value of the OPT-II problem, the
 * positive dual variables associated with the walk constraints, and the
 * optimal edge costs obtained from the cutting-plane procedure.
 */
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
 * @brief Solves the OPT-II problem using a cutting-plane procedure.
 *
 * @param n Number of nodes.
 * @param x Input fractional solution, represented as a map from edges to
 *          their values. Only edges with positive value are included in the
 *          LP.
 * @param starting_walks Initial collection of walks whose corresponding
 *                       constraints are included in the LP before the
 *                       cutting-plane procedure starts.
 * @param tol Numerical tolerance used to determine whether the Graph-TSP
 *            value is sufficiently close to one and the "non-zero"
 *            conditions of the dual variables
 * @param verbosity Controls the amount of information printed during the
 *                  computation. Higher values produce more detailed output.
 *
 * @return An OptIISolution containing the optimal value and solution
 *         information.
 */
OptIISolution solve_optII(
    int n,
    const std::map<std::pair<int, int>, double>& x,
    const std::vector<Walk>& starting_walks,
    double tol,
    int verbosity
);

/**
 * @brief Runs the Gap-Bounding (GB) algorithm on a given fractional solution. It runs just a single iteration.
 *
 * @param n Integer, Number of nodes.
 * @param x0 Vertex, Input fractional solution, represented as a map from edges to
 *           their values.
 * @param starting_walks Walk, Initial collection of walks used by the OPT-II
 *                       procedure.
 * @param tol Double, Numerical tolerance used when testing whether an edge has value
 *            one and when checking the OPT-II value.
 * @param verbosity Integer, Controls the amount of information printed during the
 *                  computation. A value of 0 suppresses informational output,
 *                  while larger values provide progressively more details.
 *
 * @return  GBAlgorithmResult object.
 */
GBAlgorithmResult GB_algorithm(
    int n,
    const map<pair<int,int>, double>& x0,
    const vector<Walk>& starting_walks = {},
    double tol = 1e-6,
    int verbosity = 1
);

/**
 * @brief Our main Gap Bounding Algorithm
 *
 * @param n Integer, number of nodes
 * @param x0 Vertex, the ancestor we wish to expand
 * @param target_gapII, Double, Target gap that will end our algorithm
 * @param max_iterations, Integer, Maximum number of iterations allowed
 * @param tol, Double, Numeric tolerance for considering the value of the GB algorithm below the threshold
 * @param verbosity Integer, Controls the amount of information printed during the
 * computation. A value of 0 suppresses informational output, while larger values provide progressively more details.

 * @return GBAlgorithmResult, with all the infos on the algorithm
 */
GBAlgorithmResult GBe_algorithm(
    int n,
    const Vertex& x0,
    double target_gapII,
    int max_iterations,
    double tol,
    int verbosity
);

#endif