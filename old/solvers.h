#ifndef SOLVERS_H
#define SOLVERS_H

#include <vector>
#include <tuple>

using namespace std;

// Only the signature (declaration)
std::tuple<double, std::vector<int>> solve_tsp_concorde(const std::vector<int>& C, int ncount, int silent);
std::tuple<double, std::vector<int>> solve_tsp_silent(const std::vector<int>& C, int ncount);

// SCIP accept floats :)
std::tuple<double, std::vector<pair<int, int>>> solve_tsp_scip(const std::vector<double>& C, int ncount, int verbosity);


#endif