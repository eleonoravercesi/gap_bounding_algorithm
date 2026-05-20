#include <tuple>
#include <string>
// Include SCIP headers
#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include "solvers.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <format>
#include <iostream>   // ← cout, endl
#include <iomanip>    // ← setprecision
using namespace std;


int formula_i_j_e(int i, int j, int n) {
    int A =  2*n*i - i * i - 3 * i + 2 * j;
    return (A / 2) -1 ;
}

/**
 * Enumerates all unique Hamiltonian cycles in a symmetric complete graph.
 * @param n_nodes Number of nodes in the graph.
 * @return A vector of tours (each tour is a sequence of node indices).
 */
std::vector<std::vector<int>> enumerate_all_tsp_tours(int n_nodes) {
    std::vector<std::vector<int>> all_tours;
    if (n_nodes < 3) return all_tours;

    // Create a list of nodes excluding the start node (0)
    std::vector<int> nodes(n_nodes - 1);
    std::iota(nodes.begin(), nodes.end(), 1); // Fill with 1, 2, ..., n-1

    // We use std::next_permutation to generate all orderings of nodes {1...n-1}
    do {
        /*
         * Symmetry Breaking:
         * A tour 0 -> v1 -> ... -> vk -> 0 is the same as 0 -> vk -> ... -> v1 -> 0.
         * To only keep one, we ensure the first node visited (nodes[0])
         * is smaller than the last node visited (nodes.back()).
         */
        if (nodes[0] < nodes.back()) {
            std::vector<int> current_tour;
            current_tour.push_back(0); // Start at 0
            for (int node : nodes) {
                current_tour.push_back(node);
            }
            // current_tour.push_back(0); // Optional: Close the tour
            all_tours.push_back(current_tour);
        }
    } while (std::next_permutation(nodes.begin(), nodes.end()));

    return all_tours;
}


std::pair<long long, long long> toFraction(double x, double tol = 1e-5) {
    /**
    * Claude did this
    **/

    x = std::abs(x);

    // Continued fraction: track convergents h/k
    long long h1 = 1, h2 = 0;   // numerator convergents
    long long k1 = 0, k2 = 1;   // denominator convergents
    double b = x;

    while (true) {
        long long a = static_cast<long long>(b);

        long long h = a * h1 + h2;
        long long k = a * k1 + k2;

        if (k != 0 && std::abs(x - static_cast<double>(h) / k) < tol)
            return {h, k };

        h2 = h1;  h1 = h;
        k2 = k1;  k1 = k;

        b = 1.0 / (b - a);
        if (std::isinf(b)) break;   // x was an exact integer
    }

    return { h1, k1 };
}

double opt_plus(const vector<double>& x, int n) {
    // Setting this was hard
    double TOL = 5e-4; // Almost 10-3
    double TOL_rounding = 1e-5;
    SCIP* scip = nullptr;
    SCIP_CALL(SCIPcreate(&scip));
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));
    SCIP_CALL(SCIPcreateProbBasic(scip, "OPT_Model"));

    // Suppress output for performance
    SCIPsetIntParam(scip, "display/verblevel", 0);

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


    // Create Variables (c_e)
    std::vector<SCIP_VAR*> c_vars(n_edges);
    for (int e = 0; e < n_edges; ++e) {
        char vname[32];
        snprintf(vname, sizeof(vname), "c_%d_%d", edges[e].u, edges[e].v);
        // Objective is sum(x[e] * c[e])
        SCIP_CALL(SCIPcreateVarBasic(scip, &c_vars[e], vname, 0.0, SCIPinfinity(scip), x[e], SCIP_VARTYPE_CONTINUOUS));
        SCIP_CALL(SCIPaddVar(scip, c_vars[e]));
    }

    // 3. Triangle Inequality Constraints
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            for (int k = j + 1; k < n; ++k) {
                int ij = formula_i_j_e(i, j, n);
                int ik = formula_i_j_e(i, k, n);
                int jk = formula_i_j_e(j, k, n);

                int indices[3][3] = {{ij, ik, jk}, {ik, ij, jk}, {jk, ij, ik}};
                for (int m = 0; m < 3; ++m) {
                    SCIP_CONS* cons = nullptr;
                    // c[a] - c[b] - c[c] <= 0  ==> c[a] <= c[b] + c[c]
                    const char* c_name = format("tri_{}_{}_{}", indices[m][0], indices[m][1], indices[m][2]).c_str();
                    SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, c_name, 0, nullptr, nullptr, -SCIPinfinity(scip), 0.0));
                    SCIP_CALL(SCIPaddCoefLinear(scip, cons, c_vars[indices[m][0]], 1.0));
                    SCIP_CALL(SCIPaddCoefLinear(scip, cons, c_vars[indices[m][1]], -1.0));
                    SCIP_CALL(SCIPaddCoefLinear(scip, cons, c_vars[indices[m][2]], -1.0));
                    SCIP_CALL(SCIPaddCons(scip, cons));
                    SCIP_CALL(SCIPreleaseCons(scip, &cons));
                }
            }
        }
    }

    double tsp_val = 0.0;


    // Cutting Plane Loop
    int cont = 0;
    while (true) {
        SCIP_CALL(SCIPsolve(scip));
        SCIP_SOL* sol = SCIPgetBestSol(scip);
        if (!sol) break;

        // Get SCIP best solution
        // double objval = SCIPgetSolOrigObj(scip, sol);
        // DEBUG
        // cout << "Obj = " << objval << endl;

        std::vector<double> c_star(n_edges);

        cout << setprecision(15);
        for (int e = 0; e < n_edges; ++e) {
            c_star[e] = SCIPgetSolVal(scip, sol, c_vars[e]);
        }

        vector<double> c_star_rounded(n_edges);
        for (int e = 0; e < n_edges; ++e) {
            c_star_rounded[e] = round(SCIPgetSolVal(scip, sol, c_vars[e]) * (1/TOL_rounding)) / (1/TOL_rounding);

            // Can be negative 🆘
            if (c_star_rounded[e] <= 0) {
                c_star_rounded[e] = 0;
            }
        }

        //DEBUG
        // for (int e = 0; e < n_edges; ++e) {
        //     cout << c_star_rounded[e] << ",";
        // }
        // cout << endl;

        auto tsp_result = solve_tsp_scip(c_star_rounded, n, 0);
        tsp_val = get<0>(tsp_result);
        vector<pair<int, int>> edges_t;
        edges_t = get<1>(tsp_result); // Indicator vector (1 if edge in tour, 0 otherwise)

        // Convergence Check
        if (tsp_val >= 1 - TOL) break;

        // Add violated constraint: sum(t_star[e] * c[e]) >= 1
        SCIP_CALL(SCIPfreeTransform(scip)); // Move back to problem stage to add constraints
        SCIP_CONS* tour_cons = nullptr;
        const char* c_name = format("tour_cut_{}", cont).c_str();
        cont++;
        SCIP_CALL(SCIPcreateConsBasicLinear(scip, &tour_cons, c_name, 0, nullptr, nullptr,  1, SCIPinfinity(scip)));

        for (int i = 0; i < n; ++i) {
            int u = edges_t[i].first;
            int v = edges_t[i].second;
            int e = formula_i_j_e(u, v, n);
            SCIP_CALL(SCIPaddCoefLinear(scip, tour_cons, c_vars[e], 1));
        }
        SCIP_CALL(SCIPaddCons(scip, tour_cons));
        SCIP_CALL(SCIPreleaseCons(scip, &tour_cons));


        // Write the problem as it stands in the "Original" space
        // SCIP_CALL(SCIPwriteOrigProblem(scip, "my_model.lp", NULL, FALSE) );

    }

    // Final re-optimization
    SCIP_CALL(SCIPsolve(scip));
    double final_opt = SCIPgetPrimalbound(scip);

    // Cleanup
    for (int e = 0; e < n_edges; ++e) SCIP_CALL(SCIPreleaseVar(scip, &c_vars[e]));
    SCIP_CALL(SCIPfree(&scip));


    return final_opt;
}


// double opt_plus_concorde(const vector<double>& x, int n) {
//
//     SCIP* scip = nullptr;
//     SCIP_CALL(SCIPcreate(&scip));
//     SCIP_CALL(SCIPincludeDefaultPlugins(scip));
//     SCIP_CALL(SCIPcreateProbBasic(scip, "OPT_Model"));
//
//     // Suppress output for performance
//     SCIPsetIntParam(scip, "display/verblevel", 0);
//
//     // Create Edges Mapping (i < j)
//     struct Edge { int u, v; };
//     std::vector<Edge> edges;
//     for (int i = 0; i < n; ++i) {
//         for (int j = i + 1; j < n; ++j) {
//             edges.push_back({i, j});
//         }
//     }
//     int n_edges = edges.size();
//     assert(n_edges == n * (n - 1) / 2);
//
//
//     // Create Variables (c_e)
//     std::vector<SCIP_VAR*> c_vars(n_edges);
//     for (int e = 0; e < n_edges; ++e) {
//         char vname[32];
//         snprintf(vname, sizeof(vname), "c_%d_%d", edges[e].u, edges[e].v);
//         // Objective is sum(x[e] * c[e])
//         SCIP_CALL(SCIPcreateVarBasic(scip, &c_vars[e], vname, 0.0, SCIPinfinity(scip), x[e], SCIP_VARTYPE_CONTINUOUS));
//         SCIP_CALL(SCIPaddVar(scip, c_vars[e]));
//     }
//
//     // 3. Triangle Inequality Constraints
//     for (int i = 0; i < n; ++i) {
//         for (int j = i + 1; j < n; ++j) {
//             for (int k = j + 1; k < n; ++k) {
//                 int ij = formula_i_j_e(i, j, n);
//                 int ik = formula_i_j_e(i, k, n);
//                 int jk = formula_i_j_e(j, k, n);
//
//                 int indices[3][3] = {{ij, ik, jk}, {ik, ij, jk}, {jk, ij, ik}};
//                 for (int m = 0; m < 3; ++m) {
//                     SCIP_CONS* cons = nullptr;
//                     // c[a] - c[b] - c[c] <= 0  ==> c[a] <= c[b] + c[c]
//                     const char* c_name = format("tri_{}_{}_{}", indices[m][0], indices[m][1], indices[m][2]).c_str();
//                     SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, c_name, 0, nullptr, nullptr, -SCIPinfinity(scip), 0.0));
//                     SCIP_CALL(SCIPaddCoefLinear(scip, cons, c_vars[indices[m][0]], 1.0));
//                     SCIP_CALL(SCIPaddCoefLinear(scip, cons, c_vars[indices[m][1]], -1.0));
//                     SCIP_CALL(SCIPaddCoefLinear(scip, cons, c_vars[indices[m][2]], -1.0));
//                     SCIP_CALL(SCIPaddCons(scip, cons));
//                     SCIP_CALL(SCIPreleaseCons(scip, &cons));
//                 }
//             }
//         }
//     }
//
//     double tsp_val = 0.0;
//
//     int cont = 0;
//     int MAX_ITER = 10;
//
//     // Cutting Plane Loop
//     while (true) {
//         SCIP_CALL(SCIPsolve(scip));
//         SCIP_SOL* sol = SCIPgetBestSol(scip);
//         if (!sol) break;
//
//         // Get SCIP best solution
//         double objval = SCIPgetSolOrigObj(scip, sol);
//         // cout << "Obj = " << objval << endl;
//
//         std::vector<double> c_star(n_edges);
//
//         // cout << setprecision(15);
//         for (int e = 0; e < n_edges; ++e) {
//             c_star[e] = SCIPgetSolVal(scip, sol, c_vars[e]);
//             // cout << c_star[e] << " ";
//         }
//         // cout << endl;
//
//         // Thing one: from c_star, det the GDG
//         long long MEGA = 1;
//         for (double c_ij : c_star) {
//             auto [num, den] = toFraction(c_ij);
//             MEGA = std::lcm(MEGA, den);
//
//             // Just to print
//             int num_int = num;
//             int den_int = den;
//            // cout << num_int << "/" << den_int << "    ";
//         }
//
//         // Initialize a vector of vector of int
//         vector<vector<int>> C_int(n, vector<int>(n, 0));
//         int MEGA_int = MEGA;
//         // cout << MEGA_int << endl;
//         int cont = 0;
//         for (int i = 0; i < n; i ++) {
//             for (int j = i + 1; j < n; j++) {
//                 double c_ij = c_star[cont];
//                 auto [num, den] = toFraction(c_ij);
//                 int c_ij_int = MEGA_int * num / den;
//                 cont ++;
//                 C_int[i][j] = c_ij_int;
//                 C_int[j][i] = c_ij_int; // Symmetric
//             }
//         }
//
//         vector<int> C;
//         for (int i = 0; i < n; ++i) {
//             for (int j = 0; j < i; j ++) {
//                 C.push_back(C_int[i][j]);
//             }
//         }
//
//         assert(C.size() == n_edges);
//
//
//         auto tsp_result = solve_tsp_silent(C, n);
//         tsp_val = get<0>(tsp_result);
//         std::vector<int> t_star(n);
//         t_star = get<1>(tsp_result); // Indicator vector (1 if edge in tour, 0 otherwise)
//         // Add the first node in the last place
//         t_star.push_back(t_star[0]);
//
//         // Convergence Check
//         if (tsp_val >= MEGA_int) break; // Consider rounding errorrrrrrs
//
//         // Add violated constraint: sum(t_star[e] * c[e]) >= 1
//         SCIP_CALL(SCIPfreeTransform(scip)); // Move back to problem stage to add constraints
//         SCIP_CONS* tour_cons = nullptr;
//         const char* c_name = format("tour_cut_{}", cont).c_str();
//         SCIP_CALL(SCIPcreateConsBasicLinear(scip, &tour_cons, c_name, 0, nullptr, nullptr,  1, SCIPinfinity(scip)));
//
//         for (int i = 0; i < n; ++i) {
//             int u = t_star[i];
//             int v = t_star[(i + 1)];
//             // I want u < v
//             if (u > v) {
//                 std::swap(u, v);
//             }
//             int e = formula_i_j_e(u, v, n);
//             SCIP_CALL(SCIPaddCoefLinear(scip, tour_cons, c_vars[e], 1));
//         }
//         SCIP_CALL(SCIPaddCons(scip, tour_cons));
//         SCIP_CALL(SCIPreleaseCons(scip, &tour_cons));
//
//         // Write the problem as it stands in the "Original" space
//         // SCIP_CALL(SCIPwriteOrigProblem(scip, "my_model.lp", NULL, FALSE) );
//
//     }
//
//     // Final re-optimization
//     SCIP_CALL(SCIPsolve(scip));
//     double final_opt = SCIPgetPrimalbound(scip);
//
//     // Cleanup
//     for (int e = 0; e < n_edges; ++e) SCIP_CALL(SCIPreleaseVar(scip, &c_vars[e]));
//     SCIP_CALL(SCIPfree(&scip));
//
//
//     return final_opt;
// }