#include "opt_hat.h"

#include <ranges>

#include "graph_utils.h"
#include "gurobi_c++.h"

using std::map, std::pair, std::vector;

constexpr double TOLERANCE = 1e-6;


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


GTSPSolution solve_gtsp(const Cost& c, const vector<Edge>& forced_edges = {}) {
    vector<Edge> edges;
    for (const Edge& e : c | std::views::keys)
        edges.push_back(e);

    int n = 0;
    for (auto& [i, j] : edges)
        n = std::max(n, std::max(i, j));
    ++n;

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model = GRBModel(env);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_IntParam_LazyConstraints, 1);

    // Variables
    map<pair<int, int>, GRBVar> w_vars;
    for (Edge& e : edges)
        w_vars[e] = model.addVar(0.0, 2.0, 0.0, GRB_INTEGER,
            std::format("w_{}_{}", e.first, e.second));
    for (const Edge& e : forced_edges)
        w_vars[e].set(GRB_DoubleAttr_LB, 1.0);

    vector<GRBVar> d_vars(n);
    for (int i = 0; i < n; ++i)
        d_vars[i] = model.addVar(1.0, n - 1, 0.0, GRB_INTEGER,
            std::format("d_{}", i));

    model.update();

    // Objective
    GRBLinExpr obj = 0;
    for (auto& [e, w_var] : w_vars)
        obj += c.at(e) * w_var;
    model.setObjective(obj, GRB_MINIMIZE);

    // Constraints
    GRBLinExpr degree_expr;
    for (int i = 0; i < n; ++i) {
        degree_expr = 0;
        for (auto& [e, w_var] : w_vars)
            if (e.first == i || e.second == i)
                degree_expr += w_var;
        model.addConstr(degree_expr == 2.0 * d_vars[i],
            std::format("degree_{}", i));
    }

    // Callback
    ConnectivityCutCallback callback(n, w_vars);
    model.setCallback(&callback);

    // Optimize
    model.optimize();

    map<Edge, int> opt_walk;
    for (auto& [e, w_var] : w_vars) {
        double val = w_var.get(GRB_DoubleAttr_X);
        opt_walk[e] = val < 0.5 ? 0 : val < 1.5 ? 1 : 2;
    }

    return {model.get(GRB_DoubleAttr_ObjVal), opt_walk, model.get(GRB_DoubleAttr_Runtime)};
}


OptHatSolution solve_opt_hat(const Vertex& x) {
    vector<Edge> edges, one_edges;
    for (const Edge& e : x | std::views::keys) {
        if (x.at(e) > 0.0)
            edges.push_back(e);
        if (x.at(e) > 1.0 - TOLERANCE)
            one_edges.push_back(e);
    }

    int n = 0;
    for (auto& [i, j] : edges)
        n = std::max(n, std::max(i, j));
    ++n;

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model = GRBModel(env);
    model.set(GRB_IntParam_OutputFlag, 0);

    // Variables
    map<pair<int, int>, GRBVar> c_vars;
    for (Edge& e : edges)
        if (x.at(e) > 0.0)
            c_vars[e] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS,
                std::format("x_{}_{}", e.first, e.second));

    model.update();

    // Objective
    GRBLinExpr obj = 0;
    for (auto& [e, c_var] : c_vars)
        obj += x.at(e) * c_var;
    model.setObjective(obj, GRB_MINIMIZE);

    // Optimize
    Walk dummy_walk = {};
    for (Edge& e : edges)
        dummy_walk[e] = 2;
    GTSPSolution gtsp_sol = {0.0, dummy_walk, 0.0};

    Cost opt_cost;
    while (gtsp_sol.opt_value < 1.0 - TOLERANCE) {
        GRBLinExpr walk_expr = 0;
        for (auto& [e, c_var] : c_vars)
            walk_expr += gtsp_sol.opt_walk.at(e) * c_var;
        model.addConstr(walk_expr >= 1.0);

        model.optimize();

        for (auto& [e, c_var] : c_vars)
            opt_cost[e] = c_var.get(GRB_DoubleAttr_X);

        gtsp_sol = solve_gtsp(opt_cost, one_edges);
    }

    return {model.get(GRB_DoubleAttr_ObjVal), opt_cost, gtsp_sol.opt_walk, model.get(GRB_DoubleAttr_Runtime)};
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
                x[{i, j}] = x_mat[i][j];

    return x;
}