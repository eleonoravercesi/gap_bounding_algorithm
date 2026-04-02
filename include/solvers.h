#ifndef SOLVERS_H
#define SOLVERS_H

#include <vector>
#include <tuple>

// Only the signature (declaration)
std::tuple<double, std::vector<int>> solve_tsp(const std::vector<int>& C, int ncount, int silent);

#endif