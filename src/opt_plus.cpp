//*
//This code has been created with the help of Gurobot
//
#include "opt_plus.h"
#include <string>
#include <vector>
#include <iostream>
#include "gurobi_c++.h"
#include "solvers.h"
#include "utils.h"
#include <sstream>

using namespace std;


/**
 * Solves the Traveling Salesman Problem
 *
 * @param x Vertex
 *          Order: (0,1), (0,2), ..., (0,n-1), (1,2), (1,3), ..., (n-2,n-1)
 * @param n Number of nodes
 * @param verbosity Verbosity level:
 *          0 --> Completely silent
 *          1 --> Only custom printing
 *          2 --> Custom print and Gurobi opt log, TSP is silent
 *          3 --> Everything + TSP verbosity = 1
 *          4 --> Everything + TSP verbosity = 2
 * @return TSPSolution containing tour edges and total cost
 */
double opt_plus(const vector<double>& x, int n, int verbosity) {
    // TODO ugly
    double TOL = 1e-6;
    GRBEnv *env = NULL;
    GRBVar **vars = NULL;
    double objVal = 0.0;

    // Get the size of x
    int m = x.size();

    // Verify input size
    if (m != n * (n - 1) / 2) {
        cerr << "Error: Input vector size mismatch. Expected "
             << n * (n - 1) / 2 << " but got " << m << endl;
        return -1.0;
    }

    // Allocate 2D array for variables
    vars = new GRBVar*[n];
    for (int i = 0; i < n; i++)
        vars[i] = new GRBVar[n];

    try {
        // **UPDATED: Create empty environment**
        env = new GRBEnv(true);

        // **UPDATED: Set output flag on environment before starting**
        if (verbosity < 2) {
            env->set(GRB_IntParam_OutputFlag, 0);
        }

        // **UPDATED: Start environment (now silently)**
        env->start();

        GRBModel model = GRBModel(*env);

        // Create continuous decision variables for upper triangle only
        int cont = 0;
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                vars[i][j] = model.addVar(0.0, GRB_INFINITY, x[cont],
                                          GRB_CONTINUOUS, "c_"+itos(i)+"_"+itos(j));
                vars[j][i] = vars[i][j];  // Symmetric: vars[j][i] points to same variable
                cont++;
            }
        }

        // Triangle inequality constraints
        // For all distinct i, j, k: c[i][j] <= c[i][k] + c[j][k]
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    for (int k = 0; k < n; k++) {
                        if (k != j && k != i) {
                            model.addConstr(vars[i][j] <= vars[i][k] + vars[j][k],
                                          "ti_"+itos(i)+"_"+itos(j)+"_"+itos(k));
                        }
                    }
                }
            }
        }

        if (verbosity >= 1) {
            cout << "Initial optimization..." << endl;
        }

        // Initial optimization
        model.optimize();

        // Check if initial solve was successful
        if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
            cerr << "Error: Initial optimization did not find optimal solution" << endl;
            throw runtime_error("Optimization failed");
        }

        // Get the current C from optimized variables
        vector<double> C(m);
        cont = 0;
        for (int i = 0; i < n; i++) {  // FIX: Properly declare i
            for (int j = i + 1; j < n; j++) {
                C[cont] = vars[i][j].get(GRB_DoubleAttr_X);
                cont++;
            }
        }

        // Solve TSP with current costs
        int verb_tsp = max(0, verbosity - 2);

        TSPSolution solution = solve_tsp(n, C, verb_tsp);

        if (!solution.success) {
            cerr << "Error: TSP solver failed" << endl;
            throw runtime_error("TSP solve failed");
        }

        // Get the TSP tour value
        double tsp_val = solution.tour_value;

        if (verbosity >= 1) {
            cout << "Initial TSP tour value: " << tsp_val << endl;
        }

        int iteration = 1;
        while (tsp_val < 1.0 -  TOL) {
            if (verbosity >= 1) {
                cout << "Iteration " << iteration << ": Adding tour constraint (value = "
                     << tsp_val << ")" << endl;
            }

            vector<pair<int, int>> tour_edges = solution.tour_edges;  // FIX: Use solution

            // Add constraint: sum of edge costs in this tour >= 1
            GRBLinExpr expr = 0;  // FIX: Initialize properly
            for (size_t e_idx = 0; e_idx < tour_edges.size(); e_idx++) {  // FIX: Rename to avoid conflict
                pair<int, int> edge = tour_edges[e_idx];
                int i_node = edge.first;
                int j_node = edge.second;

                // Get the index in the upper triangle vector
                int e_pos = from_i_j_to_e(i_node, j_node, n);  // FIX: Different variable name

                // Use the variable directly
                if (i_node < j_node) {
                    expr += vars[i_node][j_node];
                } else {
                    expr += vars[j_node][i_node];
                }
            }
            model.addConstr(expr >= 1.0, "tour_" + itos(iteration));  // FIX: Use itos
            iteration++;

            // Re-optimize with new constraint
            model.optimize();

            if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
                cerr << "Error: Optimization failed at iteration " << iteration << endl;
                throw runtime_error("Optimization failed");
            }

            // Get updated costs
            cont = 0;
            for (int i = 0; i < n; i++) {
                for (int j = i + 1; j < n; j++) {
                    C[cont] = vars[i][j].get(GRB_DoubleAttr_X);
                    cont++;
                }
            }

            // Solve TSP again with updated costs
            solution = solve_tsp(n, C, verb_tsp);

            if (!solution.success) {
                cerr << "Error: TSP solver failed at iteration " << iteration << endl;
                throw runtime_error("TSP solve failed");
            }

            tsp_val = solution.tour_value;  // FIX: Use solution
        }

        if (verbosity >= 1) {
            cout << "Converged! Final TSP tour value: " << tsp_val << endl;
        }

        // Get final objective value
        objVal = model.get(GRB_DoubleAttr_ObjVal);

    } catch (GRBException& e) {
        cerr << "Gurobi Error " << e.getErrorCode() << ": " << e.getMessage() << endl;
        objVal = -1.0;
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
        objVal = -1.0;
    } catch (...) {
        cerr << "Unknown error during optimization" << endl;
        objVal = -1.0;
    }

    // Clean up
    if (vars != NULL) {
        for (int i = 0; i < n; i++)
            delete[] vars[i];
        delete[] vars;
    }
    if (env != NULL)
        delete env;

    return objVal;
}
