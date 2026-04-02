/*
 * A list of solvers we might need
 */
extern "C" {
#include "concorde.h"
}
#include <vector>
#include <tuple>
#include "../include/solvers.h"
#include <iostream>

using namespace std;

tuple<double, vector<int>> solve_tsp(const vector<int>& C, int ncount, int silent) {
    /**
     * Solve TSP when the input is in upper triangular form.
     *
     * @param C: A vector of integers representing the upper triangular form of the cost matrix.
     * @param ncount: Number of cities
     * @param silent: 0 = NOT SILENT (verbose), > 0 SILENT = not verbose
     *
     * @return opt: the optimal tour cost
     * @return tour: and argmin
     */
    CCdatagroup dat;
    CCutil_init_datagroup(&dat);

    // Dummy vectot
    vector<int> dummy_ret = {1};

    int nedges = ncount * (ncount - 1) / 2;
    int* edges_val = (int*)malloc(nedges * sizeof(int));

    // Fill the edges val
    int cont = 0;
    if (edges_val != NULL) {
        for (int i = 0; i < nedges; i++) {
            edges_val[i] = C[cont]; // No zero edges
            cont++;
        }

        // Convert the flat edge array into the CCdatagroup format
        CCutil_tri2dat(ncount, edges_val, &dat);
        free(edges_val);
    } else {
        cout << "Cannot fill edges list!" << endl;
        return make_tuple(1, dummy_ret); // Failure
    }

    int success;             // int (not pointer) to check status
    int foundtour;           // int (not pointer) to check if tour was found
    double timebound = 86400.0;
    int hit_timebound;
    int tour[ncount];         // Array to store the tour
    double optval;
    char* name = "tsp_test";

    // Initialize Random State
    CCrandstate rstate;
    CCutil_sprand(123, &rstate);

    // Call the solver
    int out = CCtsp_solve_dat(ncount, &dat, (int*)NULL, tour, (double*)NULL,
                              &optval, &success, &foundtour, name,
                              &timebound, &hit_timebound, silent, &rstate);

    if (foundtour) {
        std::cout << "Optimal Value: " << optval << std::endl;
        std::cout << "Tour found!" << std::endl;
    }

    CCutil_freedatagroup(&dat);
    if (out == 0) {
        vector<int> tour_vec;
        for (int i = 0; i < ncount; i++) {
            tour_vec.push_back(tour[i]);
        }
        return make_tuple(optval, tour_vec); // Success
    }
    return make_tuple(1, dummy_ret); // Failure
}