/*
 * A list of solvers we might need
 */

#include <vector>
#include <tuple>
#include "../include/solvers.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include <format>
#include <ppl.hh>

#include "utils.h"
using namespace std;


/**
 * Just a simple solver for TSP. Hopefully, it will not break.
 *      @param C: vector<double> the cost vector of size n*(n-1)/2, where C[e] is the cost of edge (i,j) for i < j. Lex order
 *      @param n : int, the nuber of nodes
 *      @param verbosity: int,  = 0 --> Quite, > 0, verbpse
 *
 *      @return tps_vale: double, the tsp value
 *      @return tour: vector<int> containing the nodes
 * */
tuple<double, vector<pair<int, int>>> solve_tsp_scip(const vector<double>& C, int n, int verbosity) {
    double TOL = 0.000001;
    SCIP* scip = nullptr;
    SCIP_CALL_ABORT(SCIPcreate(&scip));
    SCIP_CALL_ABORT(SCIPincludeDefaultPlugins(scip));
    SCIP_CALL_ABORT(SCIPcreateProbBasic(scip, "TSP_solver"));

    // Should resolve tolerance issue... yes.
    // After SCIPcreateProbBasic, add:
    SCIP_CALL_ABORT(SCIPsetIntParam(scip, "presolving/maxrounds", 0)); // disable presolve
    SCIP_CALL_ABORT(SCIPsetIntParam(scip, "lp/solvefreq", 1));         // solve LP at every node
    // Tighten numerical tolerances
    SCIP_CALL_ABORT(SCIPsetRealParam(scip, "numerics/feastol",    1e-9));
    SCIP_CALL_ABORT(SCIPsetRealParam(scip, "numerics/epsilon",    1e-9));
    SCIP_CALL_ABORT(SCIPsetRealParam(scip, "numerics/sumepsilon", 1e-7));
    SCIP_CALL_ABORT(SCIPsetRealParam(scip, "numerics/dualfeastol",1e-9));

    //This might save us?
    //SCIP_CALL_ABORT(SCIPsetBoolParam(scip, "propagating/symmetry/usedynamicprop", FALSE));

    // Suppress output for performance
    if (verbosity == 0) {
        SCIPsetIntParam(scip, "display/verblevel", verbosity);
    }

    // Create Edges Mapping (i < j)
    struct Edge { int u, v; };
    std::vector<Edge> edges;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            edges.push_back({i, j});
        }
    }
    int n_edges = edges.size();
    assert(n_edges == n * (n - 1) / 2);


    // Create Variables (x_e)
    std::vector<SCIP_VAR*> x_vars(n_edges);
    for (int e = 0; e < n_edges; ++e) {
        char vname[32];
        snprintf(vname, sizeof(vname), "x_%d_%d", edges[e].u, edges[e].v);
        // Objective is sum(x[e] * c[e])
        SCIP_CALL_ABORT(SCIPcreateVarBasic(scip, &x_vars[e], vname, 0.0, 1, C[e], SCIP_VARTYPE_BINARY));
        SCIP_CALL_ABORT(SCIPaddVar(scip, x_vars[e]));
    }

    // Degree constraints
    for (int i = 0; i < n; ++i) {
        const char* c_name = format("deg_{}", i).c_str();
        SCIP_CONS* cons = nullptr;
        // Collect edges incident to node i
        std::vector<SCIP_VAR*> vars;
        std::vector<double>    coefs;
        for (int e = 0; e < n_edges; ++e) {
            if (edges[e].u == i || edges[e].v == i) {
                vars.push_back(x_vars[e]);
                coefs.push_back(1.0);
            }
        }


        // lhs == rhs == 2.0 for equality
        SCIP_CALL_ABORT(SCIPcreateConsBasicLinear(
            scip, &cons, c_name,
            (int)vars.size(), vars.data(), coefs.data(),
            2.0,   // lhs
            2.0    // rhs
        ));
        SCIP_CALL_ABORT(SCIPaddCons(scip, cons));
        SCIP_CALL_ABORT(SCIPreleaseCons(scip, &cons));
    }

        // Optimize
        SCIP_CALL_ABORT(SCIPsolve(scip));
        SCIP_SOL* sol = SCIPgetBestSol(scip);

        if (!sol) {
            cerr << "Error: No feasible solution found in initial solve\n";
            for (int e = 0; e < n_edges; ++e) {
                if (x_vars[e] != nullptr) {
                    SCIP_CALL_ABORT(SCIPreleaseVar(scip, &x_vars[e]));
                }
            }
            SCIP_CALL_ABORT(SCIPfree(&scip));
            return make_tuple(0.0, vector<pair<int,int>>());
        }

        // Create adj
        vector<vector<int>> adj;
        for (int e = 0; e < n_edges; ++e) {
            if ((int)SCIPgetSolVal(scip, sol, x_vars[e]) > 0.5) {
                adj.push_back({edges[e].u, edges[e].v});
            }
        }

        // Safety check: should have exactly n edges for a valid solution
        if ((int)adj.size() != n) {
            cerr << "Error: Initial solve returned " << adj.size() << " edges instead of " << n << "\n";
            for (int e = 0; e < n_edges; ++e) {
                if (x_vars[e] != nullptr) {
                    SCIP_CALL_ABORT(SCIPreleaseVar(scip, &x_vars[e]));
                }
            }
            SCIP_CALL_ABORT(SCIPfree(&scip));
            return make_tuple(0.0, vector<pair<int,int>>());
        }

        // DEBUG
        // for (int e = 0; e < adj.size(); ++e) {
        //     cout << adj[e][0] << " " << adj[e][1] << " -- ";
        // }
        // cout << endl;


        int cont_sec = 0;
        while (true) {
            //vector<vector<int>> S_list = getComponents(adj);
            // This is stupid, but to me it is not clear which one is better;
            vector<int> S = oneComponent(adj);

            // Safety check: S should not be empty
            if (S.empty()) {
                cerr << "Error: oneComponent returned empty set\n";
                break;
            }

            vector<vector<int>> S_list = {S, {1}};

            if((int)S_list[0].size() == n) {
                break; // Tour
            }

            // Free the model once before adding new constraints
            SCIP_CALL_ABORT(SCIPfreeTransform(scip));

            // Else, add a constr for each S but the last one
            for (int s = 0; s < S_list.size() - 1; ++s) {
                vector<int> S = S_list[s];

                // // DEBUG
                // for (int q = 0; q < S.size(); ++q) {
                //     cout << S[q] << " ";
                // }
                // cout << endl;

                // Add SEC for set S
                SCIP_CONS* cons = nullptr;
                // Collect edges incident to node i
                std::vector<SCIP_VAR*> vars;
                std::vector<double>    coefs;
                for (int e = 0; e < n_edges; ++e) {
                    int u_in_S =  count(S.begin(), S.end(), edges[e].u);
                    int v_in_S =  count(S.begin(), S.end(), edges[e].v);
                    if (u_in_S +  v_in_S == 2) {
                        vars.push_back(x_vars[e]);
                        coefs.push_back(1.0);
                    }
                }


                string c_name_S = "subtour_";
                for (int i = 0; i < S.size(); ++i) {
                    c_name_S += to_string(S[i]) + "_";
                }
                cont_sec++;
                SCIP_CALL_ABORT(SCIPcreateConsBasicLinear(
                scip, &cons, c_name_S.c_str(),
                (int)vars.size(), vars.data(), coefs.data(),
                0,   // lhs
                (double)(S.size() - 1)    // rhs
            ));
                S.clear();

                // Actually add the cons
                SCIP_CALL_ABORT(SCIPaddCons(scip, cons));
                SCIP_CALL_ABORT(SCIPreleaseCons(scip, &cons));
            }

            // Solve the problem with all the |S_list| - 1 constr
            SCIP_CALL_ABORT(SCIPsolve(scip));

            // Get the best solution
            sol = SCIPgetBestSol(scip);
            if (!sol) {
                cerr << "Error: No feasible solution found after adding subtour constraints\n";
                for (int e = 0; e < n_edges; ++e) {
                    if (x_vars[e] != nullptr) {
                        SCIP_CALL_ABORT(SCIPreleaseVar(scip, &x_vars[e]));
                    }
                }
                SCIP_CALL_ABORT(SCIPfree(&scip));
                return make_tuple(0.0, vector<pair<int,int>>());
            }

           // Delete the old adj
            adj.clear();
            for (int e = 0; e < n_edges; ++e) {
                double val = SCIPgetSolVal(scip, sol, x_vars[e]);
                int rounded = (val > 0.5) ? 1 : 0;  // hard clamp, no epsilon needed
                if (rounded == 1)
                    adj.push_back({edges[e].u, edges[e].v});
            }

            if ((int)adj.size() != n) {
                cerr << "Bad solution: " << adj.size() << " edges expected " << n << "\n";
                break;  // Exit loop if solution is malformed
            }

            // DEBUG
            // for (int e = 0; e < adj.size(); ++e) {
            //     cout << adj[e][0] << " " << adj[e][1] << " -- ";
            // }
            // cout << endl;

            // Call the function and store the result
            SCIPwriteOrigProblem(scip, "my_model_tsp.lp", "lp", FALSE);


            // DEBUG
            // for (int e = 0; e < n_edges; ++e) {
            //         cout << C[e] << ", ";
            //     }
            //     cout << endl;

            // DEBUG
            // for (int e = 0; e < n_edges; ++e) {
            //     // Get the value of the VARIABLE
            //     cout << SCIPgetSolVal(scip, sol, x_vars[e]) << " ";
            // }
            // cout << endl;

        }

        // Write the model so far
        // SCIP_CALL_ABORT( SCIPwriteOrigProblem(scip, "my_model_tsp.lp", "lp", FALSE) );


        // At the end of the day, we should hve a tour
        // I want the tour and the TSP value
        sol = SCIPgetBestSol(scip);
        double objval = 0.0;
        vector<pair<int,int>> edges_here;

        if (sol) {
            objval = SCIPgetSolOrigObj(scip, sol); // Easy
            for (int e = 0; e < n_edges; ++e) {
                if (SCIPgetSolVal(scip, sol, x_vars[e]) >= 1 - TOL) {
                    edges_here.emplace_back(edges[e].u, edges[e].v);
                }
            }

            // Sanity check: tour should have exactly n edges
            if ((int)edges_here.size() != n) {
                cerr << "Warning: solve_tsp_scip returning " << edges_here.size() << " edges instead of expected " << n << "\n";
                edges_here.clear();
                objval = 0.0;
            }
        } else {
            // No feasible solution: return empty tour and obj 0
            objval = 0.0;
        }

        // Cleanup: release variables and free SCIP
        for (int e = 0; e < n_edges; ++e) {
            if (x_vars[e] != nullptr) {
                SCIP_CALL_ABORT(SCIPreleaseVar(scip, &x_vars[e]));
            }
        }

        SCIP_CALL_ABORT(SCIPfree(&scip));

        return make_tuple(objval, edges_here);
}
