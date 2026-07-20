#include "opt_hat.h"

#include <ranges>

#include "graph_utils.h"
#include "gurobi_c++.h"

using std::map, std::pair, std::vector;


class ConnectivityCutCallback : public GRBCallback {
    const int n;
    const map<pair<int, int>, GRBVar>& w_vars;

public:
    ConnectivityCutCallback(int n, const map<pair<int, int>, GRBVar>& w_vars)
        : n(n), w_vars(w_vars) {}

protected:
    void callback() override {
        try {
            if (where != GRB_CB_MIPSOL)
                return;

            vector<Edge> edges, support_edges;
            map<int, vector<Edge>> adj;
            for (auto& e : w_vars | std::views::keys) {
                edges.push_back(e);
                if (getSolution(w_vars.at(e)) > 0.5)
                    support_edges.push_back(e);
                adj[e.first].push_back(e);
                adj[e.second].push_back(e);
            }

            vector<vector<int>> components = connected_components(n, support_edges);
            if (components.size() > 1) {
                vector<char> in_comp(n, 0);
                for (auto& comp : components) {
                    for (int node : comp)
                        in_comp[node] = 1;

                    GRBLinExpr cut_expr = 0;
                    for (int node : comp)
                        for (auto& e : adj[node])
                            if (!in_comp[e.first] || !in_comp[e.second])
                                cut_expr += w_vars.at(e);

                    addLazy(cut_expr >= 2.0);

                    for (int node : comp)
                        in_comp[node] = 0;
                }
            }

        }
        catch (GRBException& e) {
            std::cerr << "Callback error: " << e.getMessage() << std::endl;
        }
    }
};


GTSPSolution solve_gtsp(const Cost& c) {
    vector<Edge> edges;
    for (auto& e : c | std::views::keys)
        edges.push_back(e);

    int n = 0;
    for (auto& [i, j] : edges)
        n = std::max(n, std::max(i, j));
    ++n;

    try {
        GRBEnv env = GRBEnv(true);
        env.set(GRB_IntParam_OutputFlag, 0);
        env.start();

        GRBModel model = GRBModel(env);
        model.set(GRB_IntParam_OutputFlag, 0);
        model.set(GRB_IntParam_LazyConstraints, 1);

        // Variables
        map<pair<int, int>, GRBVar> w_vars;
        for (Edge e : edges)
            w_vars[e] = model.addVar(0.0, 2.0, 0.0, GRB_INTEGER,
                "w_" + std::to_string(e.first) + "_" + std::to_string(e.second));

        vector<GRBVar> d_vars(n);
        for (int i = 0; i < n; ++i)
            d_vars[i] = model.addVar(1.0, 2 * (n - 1), 0.0, GRB_INTEGER,
                "d_" + std::to_string(i));

        model.update();

        // Objective
        GRBLinExpr obj = 0;
        for (auto& [e, w_var] : w_vars)
            obj += c.at(e) * w_var;
        model.setObjective(obj, GRB_MINIMIZE);

        // Constraints
        for (int i = 0; i < n; ++i) {
            GRBLinExpr expr = 0;
            for (auto& [e, w_var] : w_vars) {
                if (e.first == i || e.second == i)
                    expr += w_var;
            }
            model.addConstr(expr >= 2.0 * d_vars[i], "degree_" + std::to_string(i));
        }

        // Callback
        ConnectivityCutCallback callback(n, w_vars);
        model.setCallback(&callback);

        model.optimize();

        map<Edge, int> opt_walk;
        for (auto& [e, w_var] : w_vars) {
            double val = w_var.get(GRB_DoubleAttr_X);
            opt_walk[e] = val < 0.5 ? 0 : val < 1.5 ? 1 : 2;
        }

        return {model.get(GRB_DoubleAttr_ObjVal), opt_walk, model.get(GRB_DoubleAttr_Runtime)};
    }
    catch (const GRBException& e) {
        std::cerr << "Gurobi Error: " << e.getMessage() << std::endl;
        throw;
    }
}


OptHatSolution solve_opt_hat(const Vertex& x) {
    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model = GRBModel(env);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_IntAttr_ModelSense, 1);  // minimization

    for (auto& [edge, value] : x)
        if (value > 0)
            model.addVar(0.0, 1.0, value, GRB_CONTINUOUS,
                "c_" + std::to_string(edge.first) + "_" + std::to_string(edge.second));

    // model.addConstr(??? >= 1.0);


    model.optimize();



    // try {
    //     ...
    // } catch (const GRBException& e) {
    //     std::cerr << "Gurobi Error: " << e.getMessage() << std::endl;
    //     throw;
    // }

    return {};
}


Vertex tetrahedron_instance() {
    Vertex x = Vertex();
    vector<vector<double>> x_mat;
    x_mat.emplace_back(vector{0.0, 0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0});
    x_mat.emplace_back(vector{0.5, 0.0, 0.5, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0});
    x_mat.emplace_back(vector{0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0});
    x_mat.emplace_back(vector{1.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0});
    x_mat.emplace_back(vector{0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0});
    x_mat.emplace_back(vector{0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0});
    x_mat.emplace_back(vector{0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0});
    x_mat.emplace_back(vector{0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.0});
    x_mat.emplace_back(vector{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 1.0, 0.0});
    x_mat.emplace_back(vector{0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5});
    x_mat.emplace_back(vector{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.5, 0.0, 0.5});
    x_mat.emplace_back(vector{0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.0});

    for (int i = 0; i < 12; i++)
        for (int j = i + 1; j < 12; j++)
            if (x_mat[i][j] > 0)
                x[make_pair(i, j)] = x_mat[i][j];

    return x;
}