#ifndef MATH_PROG_EXTENDED_TEST_H
#define MATH_PROG_EXTENDED_TEST_H

#include "utils.h"


struct GTSPSolution {
    double opt_value;
    Walk opt_walk;
    double solve_time;
};

struct OptHatSolution {
    double opt_value;
    Vertex opt_cost;
    Walk opt_walk;
    double solve_time;
};

GTSPSolution solve_gtsp(const Cost& c);

OptHatSolution solve_opt_hat(const Vertex& x);

Vertex tetrahedron_instance();

#endif //MATH_PROG_EXTENDED_TEST_H
