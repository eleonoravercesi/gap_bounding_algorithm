#ifndef MATH_PROG_EXTENDED_OPT_HAT
#define MATH_PROG_EXTENDED_OPT_HAT

#include "utils.h"


struct GTSPSolution {
    double opt_value;
    Walk opt_walk;
    double solve_time;
};

struct OptHatSolution {
    double opt_value;
    Cost opt_cost;
    Walk opt_walk;
    double solve_time;
};

GTSPSolution solve_gtsp(const Cost& c, const vector<Edge>& forced_edges = {});

OptHatSolution solve_opt_hat(const Vertex& x, const vector<Edge>& contracted_edges = {});

bool is_fractional_negligible(const Vertex& x);

#endif //MATH_PROG_EXTENDED_OPT_HAT
