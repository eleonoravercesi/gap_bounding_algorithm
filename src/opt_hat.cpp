#include "opt_hat.h"

#include <ranges>

#include "graph_utils.h"
#include "gurobi_c++.h"

using std::map, std::pair, std::vector;


class ConnectivityCutCallback : public GRBCallback {
    map<pair<int, int>, GRBVar>* w_vars;
    int n;

public:
    ConnectivityCutCallback(map<pair<int, int>, GRBVar>* w_vars_ptr, int n)
        : w_vars(w_vars_ptr), n(n) {}

protected:
    void callback() override {
        try {
            if (where != GRB_CB_MIPSOL)
                return;

            vector<Edge> edges;
            for (const auto& edge : *w_vars | std::views::keys)
                edges.push_back(edge);

            vector<vector<int>> components = connected_components(n, edges);
            if (components.size() > 1) {
                for (const auto& comp : components) {
                    GRBLinExpr cut_expr = 0;
                    for (const auto& [edge, w_var] : *w_vars) {
                        auto& [i, j] = edge;
                        int count = 0;
                        for (const auto& node : comp) {
                            if (i == node || j == node)
                                if (++count == 2)
                                    break;
                        }
                        if (count == 1)
                            cut_expr += w_var;
                    }
                    addLazy(cut_expr >= 2.0);
                }
            }

        } catch (GRBException& e) {
            std::cerr << "Callback error: " << e.getMessage() << std::endl;
        }
    }
};


GTSPSolution solve_gtsp(const Cost& c) {
    vector<Edge> edges;
    for (const auto& e : c | std::views::keys)
        edges.push_back(e);

    int n = 0;
    for (const auto& [i, j] : edges)
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
        for (const auto& [e, w_var] : w_vars)
            obj += c.at(e) * w_var;
        model.setObjective(obj, GRB_MINIMIZE);

        // Constraints
        for (int i = 0; i < n; ++i) {
            GRBLinExpr expr = 0;
            for (const auto& [e, w_var] : w_vars) {
                if (e.first == i || e.second == i)
                    expr += w_var;
            }
            model.addConstr(expr >= 2.0 * d_vars[i], "degree_" + std::to_string(i));
        }

        // Callback
        ConnectivityCutCallback callback(&w_vars, n);
        model.setCallback(&callback);

        model.optimize();

        map<Edge, int> opt_walk;
        for (const auto& [e, w_var] : w_vars) {
            double val = w_var.get(GRB_DoubleAttr_X);
            opt_walk[e] = val < 0.5 ? 0 : val < 1.5 ? 1 : 2;
        }

        return {model.get(GRB_DoubleAttr_ObjVal), opt_walk, model.get(GRB_DoubleAttr_Runtime)};
    }
    catch (const GRBException& e) {
        std::cerr << "Gurobi Error: " << e.getMessage() << std::endl;
        exit(1);
    }
}


OptHatSolution solve_opt_hat(const Vertex& x) {
    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model = GRBModel(env);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_IntAttr_ModelSense, 1);  // minimization

    for (const auto& [edge, value] : x)
        if (value > 0)
            model.addVar(0.0, 1.0, value, GRB_CONTINUOUS,
                "c_" + std::to_string(edge.first) + "_" + std::to_string(edge.second));

    // model.addConstr(??? >= 1.0);


    model.optimize();



    // try {
    //     ...
    // } catch (const GRBException& e) {
    //     std::cerr << "Gurobi Error: " << e.getMessage() << std::endl;
    //     exit(1);
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