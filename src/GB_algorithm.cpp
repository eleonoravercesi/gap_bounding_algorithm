/**
 * Automatically translated from this https://github.com/tulliovilla/IG_for_TSP_with_few_edges/blob/master/optII.py#L4 by Gurobot
 * and debugged by T. Villa and E. Vercesi
 * */
#include <map>
#include <vector>
#include "gurobi_c++.h"
#include "solvers.h"
#include "GB_algorithm.h"

#include "utils.h"

using namespace std;


OptIISolution solve_optII(
    int n,
    const map<pair<int,int>, double>& x,
    const vector<Walk>& starting_walks,
    double tol,
    int verbosity
) {
    OptIISolution result;
    result.success = false;
    result.opt_value = 0.0;
    result.solve_time = 0.0;
    result.iterations = 0;

    try {
        // Create environment
        GRBEnv env = GRBEnv(true);
        if (verbosity < 2) {
            env.set(GRB_IntParam_OutputFlag, 0);
            env.set(GRB_IntParam_LogToConsole, 0);
        }
        env.start();

        GRBModel model = GRBModel(env);

        // Buch of constraints realted to walks
        vector<GRBConstr> constraint_refs;

        // Get edges where x[e] > 0
        vector<pair<int,int>> edges;
        for (const auto& [e, val] : x) {
            if (val > 0) { // Should be ok because we read them from file
                edges.push_back(e);
            }
        }

        // Variables: c[e] for each edge (continuous)
        map<pair<int,int>, GRBVar> c;
        for (const auto& e : edges) {
            c[e] = model.addVar(0.0, GRB_INFINITY, x.at(e), GRB_CONTINUOUS,
                               "c_" + to_string(e.first) + "_" + to_string(e.second));
        }

        // Store considered walks
        vector<Walk> considered_walks = starting_walks;

        // Add constraints for starting walks
        for (const auto& walk : considered_walks) {
            GRBLinExpr expr = 0;
            for (const auto& e : edges) {
                if (walk.count(e) > 0) {
                    expr += walk.at(e) * c[e]; // Better at then [ ] at checking
                }
            }
            GRBConstr constr = model.addConstr(expr >= 1, "walk_" + to_string(constraint_refs.size()));
            constraint_refs.push_back(constr);  // Store constr
        }

        // Objective: minimize sum(x[e] * c[e])
        GRBLinExpr obj = 0;
        for (const auto& e : edges) {
            obj += x.at(e) * c[e];
        }
        model.setObjective(obj, GRB_MINIMIZE);

        // Initial optimization
        model.optimize();

        if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
            cerr << "Initial optimization failed" << endl;
            return result;
        }

        // Get c_star
        map<pair<int,int>, double> c_star;
        for (const auto& e : edges) {
            c_star[e] = c[e].get(GRB_DoubleAttr_X);
        }

        // Solve Graph TSP with c_star
        GraphTSPSolution gtsp_sol = solve_graph_tsp(n, c_star, 0);
        double g_tsp = gtsp_sol.tour_value;

        // Convert GraphTSPSolution to Walk
        Walk w_star;
        for (const auto& [edge, mult] : gtsp_sol.walk_edges) {
            w_star[edge] = mult;
        }

        // Iterative cutting plane
        int iter = 0;
        while (g_tsp < 1.0 - tol) {
            iter++;

            if (verbosity >= 1) {
                cout << "Iteration " << iter << ": g_tsp = " << g_tsp
                     << " < 1, adding walk constraint" << endl;
            }

            // Add walk to considered walks
            considered_walks.push_back(w_star);

            // Add constraint: sum(w_star[e] * c[e]) >= 1
            GRBLinExpr expr = 0;
            for (const auto& e : edges) {
                if (w_star.count(e) > 0) {
                    expr += w_star[e] * c[e];
                }
            }
            GRBConstr constr = model.addConstr(expr >= 1, "walk_" + to_string(iter));
            constraint_refs.push_back(constr);  // Store reference

            // Re-optimize
            model.optimize();

            if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
                cerr << "Optimization failed at iteration " << iter << endl;
                return result;
            }

            // Update c_star
            for (const auto& e : edges) {
                c_star[e] = c[e].get(GRB_DoubleAttr_X);
            }

            // Solve Graph TSP again
            gtsp_sol = solve_graph_tsp(n, c_star, 0);
            g_tsp = gtsp_sol.tour_value;

            // Convert to walk
            w_star.clear();
            for (const auto& [edge, mult] : gtsp_sol.walk_edges) {
                w_star[edge] = mult;
            }
        }

        // Extract dual values (Pi) from constraints
        for (size_t i = 0; i < constraint_refs.size() && i < considered_walks.size(); i++) {
            double pi = constraint_refs[i].get(GRB_DoubleAttr_Pi);
            if (pi > tol) {
                result.opt_variables.push_back({considered_walks[i], pi});
            }
        }

        // Final solution
        result.success = true;
        result.opt_value = model.get(GRB_DoubleAttr_ObjVal);
        result.c_star = c_star;
        result.solve_time = model.get(GRB_DoubleAttr_Runtime);
        result.iterations = iter;

        if (verbosity >= 1) {
            cout << "\n==================================\n";
            cout << "OPT-II Solution Found\n";
            cout << "==================================\n";
            cout << "Optimal value: " << result.opt_value << endl;
            cout << "Iterations: " << result.iterations << endl;
            cout << "Active walks: " << result.opt_variables.size() << endl;
            cout << "Solve time: " << result.solve_time << " seconds\n";
            cout << "==================================\n";
        }

    } catch (GRBException& e) {
        cerr << "Gurobi Error " << e.getErrorCode() << ": " << e.getMessage() << endl;
    } catch (...) {
        cerr << "Unknown error during optimization" << endl;
    }

    return result;
}

GBAlgorithmResult GB_algorithm(
    int n,
    const map<pair<int,int>, double>& x0,
    const vector<Walk>& starting_walks,
    double tol,
    int verbosity
) {
    GBAlgorithmResult result;
    result.success = false;
    result.gapII = 0.0;
    result.family_gapII = 0.0;
    result.has_worst_edge = false;
    result.worst_one_edge = {-1, -1};
    result.iterations = 1; // In this case you just conclude with one iterations

    try {
        // Find one-edges: edges where x0[e] == 1
        vector<Edge> one_edges;
        for (const auto& [e, val] : x0) {
            if (abs(val - 1.0) < tol) {  // Check if == 1 within tolerance
                one_edges.push_back(e);
            }
        }

        if (verbosity >= 1) {
            cout << "Found " << one_edges.size() << " one-edge(s)" << endl;
        }

        // Solve OPT-II
        OptIISolution optII_sol = solve_optII(n, x0, starting_walks, tol, verbosity);

        if (!optII_sol.success) {
            cerr << "OPT-II failed" << endl;
            return result;
        }

        double optII = optII_sol.opt_value;

        if (optII <= tol) {
            cerr << "OPT-II value too small: " << optII << endl;
            return result;
        }

        // Compute gapII = 1 / optII
        double gapII = 1.0 / optII;

        if (verbosity >= 1) {
            cout << "OPT-II = " << optII << ", gapII = " << gapII << endl;
        }

        // Find worst one-edge
        Edge worst_one_edge = {-1, -1};
        double worst_factor = 0.0;

        for (const auto& one_edge : one_edges) {
            double factor = 0.0;

            // For each walk in opt_variables
            for (const auto& [w, value] : optII_sol.opt_variables) {
                // Get multiplicity of one_edge in walk w
                int mult = 0;
                if (w.count(one_edge) > 0) {
                    mult = w.at(one_edge);
                }

                // factor += value * (mult if mult > 0 else 2)
                int contrib = (mult > 0) ? mult : 2;
                factor += value * contrib;
            }

            if (verbosity >= 2) {
                cout << "  One-edge (" << one_edge.first << "," << one_edge.second
                     << "): factor = " << factor << endl;
            }

            // Track worst (highest) factor
            if (factor > worst_factor) {
                worst_one_edge = one_edge;
                worst_factor = factor;
            }
        }

        // Compute family_gapII = gapII * worst_factor
        double family_gapII = gapII * worst_factor;

        if (verbosity >= 1) {
            cout << "\n==================================\n";
            cout << "GB Algorithm Result\n";
            cout << "==================================\n";
            cout << "gapII: " << gapII << endl;
            cout << "worst_factor: " << worst_factor << endl;
            cout << "family_gapII: " << family_gapII << endl;
            if (worst_one_edge.first != -1) {
                cout << "worst_one_edge: (" << worst_one_edge.first
                     << "," << worst_one_edge.second << ")" << endl;
            }
            cout << "==================================\n";
        }

        // Fill result
        result.success = true;
        result.gapII = gapII;
        result.family_gapII = family_gapII;
        result.opt_variables = optII_sol.opt_variables;
        result.worst_one_edge = worst_one_edge;
        result.has_worst_edge = (worst_one_edge.first != -1);

    } catch (exception& e) {
        cerr << "Error in GB_algorithm: " << e.what() << endl;
    }

    return result;
}

GBAlgorithmResult GBe_algorithm(
    int n,
    const Vertex& x0,
    double target_gapII,
    int max_iterations,
    double tol,
    int verbosity
) {
    GBAlgorithmResult result;
    result.success = false;
    result.gapII = 0.0;
    result.family_gapII = 0.0;
    result.iterations = 1;

    try {
        if (verbosity >= 1) {
            cout << "\n========================================\n";
            cout << "GBe Algorithm - Initial Iteration\n";
            cout << "========================================\n";
        }

        vector<Walk> empty_walks;
        GBAlgorithmResult gb_result = GB_algorithm(n, x0, empty_walks, tol, verbosity);

        if (!gb_result.success) {
            cerr << "Initial GB_algorithm failed" << endl;
            return result;
        }

        double gapII = gb_result.gapII;
        double family_gapII = gb_result.family_gapII;
        vector<pair<Walk, double>> opt_vars = gb_result.opt_variables;
        Edge worst_one_edge = gb_result.worst_one_edge;

        if (verbosity >= 1) {
            cout << "Initial gapII: " << gapII << endl;
            cout << "Initial family_gapII: " << family_gapII << endl;
            cout << "Target gapII: " << target_gapII << endl;
        }

        int iterations = 0;
        Vertex x1 = x0;
        int nn = n;

        while (family_gapII > target_gapII + tol && iterations < max_iterations) {
            iterations++;

            if (verbosity >= 1) {
                cout << "\n========================================\n";
                cout << "GBe Algorithm - Iteration " << iterations << "\n";
                cout << "========================================\n";
                cout << "Current family_gapII: " << family_gapII << endl;
                cout << "Branching on edge: (" << worst_one_edge.first
                     << "," << worst_one_edge.second << ")" << endl;
            }

            x1 = bbmove(nn, x1, worst_one_edge);

            vector<Walk> starting_walks;
            for (const auto& [w, value] : opt_vars) {
                vector<Walk> extended_walks = extend_walk(nn, w, worst_one_edge);
                starting_walks.insert(starting_walks.end(),
                                     extended_walks.begin(),
                                     extended_walks.end());
            }

            if (verbosity >= 2) {
                cout << "Generated " << starting_walks.size()
                     << " starting walks" << endl;
            }

            nn += 1;

            gb_result = GB_algorithm(nn, x1, starting_walks, tol, verbosity);

            if (!gb_result.success) {
                cerr << "GB_algorithm failed at iteration " << iterations << endl;
                return result;
            }

            family_gapII = gb_result.family_gapII;
            opt_vars = gb_result.opt_variables;
            worst_one_edge = gb_result.worst_one_edge;

            if (verbosity >= 1) {
                cout << "New family_gapII: " << family_gapII << endl;
            }
        }

        if (family_gapII > target_gapII + tol) {
            if (verbosity >= 1) {
                cout << "\nTarget gapII not reached after "
                     << max_iterations << " iterations" << endl;
            }
            iterations = -1;
        } else {
            if (verbosity >= 1) {
                cout << "\nTarget gapII reached!" << endl;
            }
        }

        result.success = true;
        result.gapII = gapII;
        result.family_gapII = family_gapII;
        result.iterations = iterations;

        if (verbosity >= 1) {
            cout << "\n========================================\n";
            cout << "GBe Algorithm Complete\n";
            cout << "========================================\n";
            cout << "Initial gapII: " << gapII << endl;
            cout << "Final family_gapII: " << family_gapII << endl;
            cout << "Iterations: " << (iterations >= 0 ? to_string(iterations) : "FAILED") << endl;
            cout << "========================================\n";
        }

    } catch (exception& e) {
        cerr << "Error in GBe_algorithm: " << e.what() << endl;
    }

    return result;
}