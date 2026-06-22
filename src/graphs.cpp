#include "graphs.h"
#include "gurobi_c++.h"

// Check isomorphism
bool are_isomorphic_vertices(VertexFraction v1, VertexFraction v2, int n, int verbose) {
    v1.Minimalize();
    v2.Minimalize();


    if (v1.v.size() != v2.v.size()) {
        if (verbose == 1) {
            cout << "Non isomorphic bc of different size" << endl;
        }
        return false;
    }
    if (v1.den != v2.den){
        if (verbose == 1) {
            cout << "Non isomorphic bc different denominator" << endl;
        }
        return false;
    }
    if (v1.toCounter() != v2.toCounter()) if (verbose == 1) {
        cout << "Non isomorphic bc of different values" << endl;
        return false;

    }

    std::vector<std::vector<int>> V1 = v1.toMatrix(n);
    std::vector<std::vector<int>> V2 = v2.toMatrix(n);

    try {
        // Create an empty environment
        GRBEnv env = GRBEnv(true);

        // Set OutputFlag BEFORE starting the environment
        env.set(GRB_IntParam_OutputFlag, 0);

        env.start();
        // Now when you create the model, the banner will not appear
        GRBModel model = GRBModel(env);
        model.set(GRB_IntParam_OutputFlag, 0);

        // Define Permutation Matrix P
        std::vector<std::vector<GRBVar>> P(n, std::vector<GRBVar>(n));
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                P[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY,
                          "x_" + std::to_string(i) + "_" + std::to_string(j));
            }
        }

        // Sum of rows and columns == 1
        for (int i = 0; i < n; i++) {
            GRBLinExpr rowExpr = 0, colExpr = 0;
            for (int j = 0; j < n; j++) {
                rowExpr += P[i][j];
                colExpr += P[j][i];
            }
            model.addConstr(rowExpr == 1);
            model.addConstr(colExpr == 1);
        }

        // Quadratic constraints: P * V1 * P^T = V2  =>  P * V1 = V2 * P
        // It is often more efficient to write as: sum_j(P_ij * V1_jk) = sum_l(V2_il * P_lk)
        for (int i = 0; i < n; i++) {
            for (int k = 0; k < n; k++) {
                GRBQuadExpr lhs = 0, rhs = 0;
                for (int j = 0; j < n; j++) lhs += P[i][j] * V1[j][k];
                for (int l = 0; l < n; l++) rhs += V2[i][l] * P[l][k];
                model.addQConstr(lhs == rhs);
            }
        }

        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL) {       // status == 2
            return true;   // Feasible permutation found → isomorphic
        } else if (status == GRB_INFEASIBLE || status == GRB_INF_OR_UNBD) {
            if (verbose == 1) {
                cout << "Non isomorphic bc of Gurobi" << endl;
            }
            return false;// 3 or 4
        } else {
            // Timeout, numerical issues, etc.
            cerr << "Unexpected Gurobi status: " << status << endl;
            return false;  // Safe default
        }

    } catch (GRBException e) {
        std::cerr << "Gurobi Error: " << e.getMessage() << std::endl;
        exit(1);
    }
}

vector<int> filter_isomorphic_graphs(const vector<VertexFraction> graphs, int n, int verbose) {
    vector<int> result = {0};

    int n_graphs = graphs.size();
    for (int i = 1; i < n_graphs; i++) {
        VertexFraction G_i = graphs[i];

        bool found = false;
        for (int j = 0; j < result.size(); j++) {
            VertexFraction G_j = graphs[result[j]];
            if (are_isomorphic_vertices(G_i, G_j, n, verbose)) {
                found = true;
                break;  // Cleaner than setting j = result.size()
            }
        }
        if (!found) {
            result.push_back(i);
        }
    }
    return result;
}