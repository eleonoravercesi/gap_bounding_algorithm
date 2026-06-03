/*
 * This code has been created with the help of Gurobot
 * */

#ifndef OPT_PLUS_H
#define OPT_PLUS_H

#include <vector>

/**
 * @brief Solves an optimization problem with triangle inequality constraints
 *        and iteratively adds TSP tour constraints until a valid solution is found.
 *
 * @param x Input cost vector (upper triangle of cost matrix, size n*(n-1)/2)
 * @param n Number of nodes
 * @param verbosity Output control:
 *                  0 = silent
 *                  1 = minimal output
 *                  2+ = full Gurobi output
 *
 * @return Optimal objective value
 *
 * @note The function builds a model with:
 *       - Continuous variables c[i][j] for each edge
 *       - Triangle inequality constraints: c[i][j] <= c[i][k] + c[j][k]
 *       - Iteratively adds constraints to ensure TSP tour value >= 1
 */
double opt_plus(const std::vector<double>& x, int n, int verbosity);

#endif // OPT_PLUS_H
