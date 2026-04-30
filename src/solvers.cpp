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
#include <fstream>

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

    // Dummy vector
    vector<int> dummy_ret = {1};

    int nedges = ncount * (ncount - 1) / 2;
    //int* edges_val = (int*)malloc(nedges * sizeof(int));
    int*edges_val = (int*)malloc((nedges) * sizeof(int) );


    // Fill the edges val
    if (edges_val != NULL) {
        for (int i = 0; i < nedges; i++) {
            edges_val[i] = C[i];
        }

        // Convert the flat edge array into the CCdatagroup format
        cout << "Before calling tr2dat:" << endl;
        for (int i = 0; i < nedges; i++) {
            cout << "Edge " << i << ": " << edges_val[i] << endl;
        }
        CCutil_tri2dat(ncount, edges_val, &dat);
        //free(edges_val); // Commented with Saverio

        cout << "After:" << endl;
        for (int i = 0; i < nedges; i++) {
            cout << "Edge " << i << ": " << edges_val[i] << endl;
        }
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

    CCutil_writetsplib("test.tsp", ncount, &dat);

    CCutil_freedatagroup(&dat);
    CCutil_init_datagroup(&dat);

    CCutil_gettsplib("test.tsp", &ncount, &dat);

    // Call the solver
    int out = CCtsp_solve_dat(ncount, &dat, (int*)NULL, tour, (double*)NULL,
                              &optval, &success, &foundtour, name,
                              &timebound, &hit_timebound, silent, &rstate);

    if (foundtour) {
        std::cout << "Optimal Value: " << optval << std::endl;
        std::cout << "Tour found!" << std::endl;
        for (int i = 0; i < ncount; i++) {
            std::cout << tour[i] << std::endl;
        }
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

tuple<double, vector<int>> solve_tsp_silent(const vector<int>& C, int ncount) {
    /**
     * The previous function but now the stoputput is ona file
     * */

        FILE* f = fopen("tsp_output.log", "w");
        int saved_fd = dup(STDOUT_FILENO);   // save real stdout
        dup2(fileno(f), STDOUT_FILENO);      // point stdout → file

        auto out = solve_tsp(C, ncount, 0);

        fflush(stdout);
        dup2(saved_fd, STDOUT_FILENO);       // restore
        close(saved_fd);
        fclose(f);
    return out;
}