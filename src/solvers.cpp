/* This has been created starting from https://docs.gurobi.com/projects/examples/en/current/examples/cpp/tsp_c++.html
 * and using the LLM Gurobot to adapt it to my needs
 **/

#include "gurobi_c++.h"
#include <vector>
#include <iostream>
#include <cassert>
#include <sstream>
using namespace std;
#include "utils.h"
#include <map>


// Structure to hold the solution
struct TSPSolution {
    vector<pair<int, int>> tour_edges;  // List of edges in the tour
    double tour_value;                   // Total cost of the tour
    bool success;                        // Whether a solution was found
};

// Forward declarations
void findsubtour(int n, double** sol, int* tourlenP, int* tour);

// Subtour elimination callback - CORRECTED VERSION
class subtourelim: public GRBCallback
{
  public:
    GRBVar** vars;
    int n;
    subtourelim(GRBVar** xvars, int xn) {
      vars = xvars;
      n    = xn;
    }
  protected:
    void callback() {
        try {
            if (where == GRB_CB_MIPSOL) {
                double **x = new double*[n];
                int *tour = new int[n];
                int len;

                // Get solution values - now safe because diagonal exists
                for (int i = 0; i < n; i++)
                    x[i] = getSolution(vars[i], n);

                findsubtour(n, x, &len, tour);

                if (len < n) {
                    // Add subtour elimination constraint
                    GRBLinExpr expr = 0;
                    for (int i = 0; i < len; i++) {
                        for (int j = i+1; j < len; j++) {
                            int node_i = tour[i];
                            int node_j = tour[j];

                            // Use canonical order (smaller index first)
                            int u = (node_i < node_j) ? node_i : node_j;
                            int v = (node_i < node_j) ? node_j : node_i;

                            expr += vars[u][v];
                        }
                    }
                    addLazy(expr <= len-1);
                }

                for (int i = 0; i < n; i++)
                    delete[] x[i];
                delete[] x;
                delete[] tour;
            }
        } catch (GRBException& e) {
            cerr << "Callback Error " << e.getErrorCode() << ": " << e.getMessage() << endl;
        } catch (...) {
            cerr << "Unknown error during callback" << endl;
        }
    }
};

// Given an integer-feasible solution 'sol', find the smallest sub-tour
void findsubtour(int n, double** sol, int* tourlenP, int* tour)
{
  bool* seen = new bool[n];
  int bestind, bestlen;
  int i, node, len, start;

  for (i = 0; i < n; i++)
    seen[i] = false;

  start = 0;
  bestlen = n+1;
  bestind = -1;

  while (start < n) {
    for (node = 0; node < n; node++)
      if (!seen[node])
        break;
    if (node == n)
      break;

    for (len = 0; len < n; len++) {
      tour[start+len] = node;
      seen[node] = true;
      for (i = 0; i < n; i++) {
        if (sol[node][i] > 0.5 && !seen[i]) {
          node = i;
          break;
        }
      }
      if (i == n) {
        len++;
        if (len < bestlen) {
          bestlen = len;
          bestind = start;
        }
        start += len;
        break;
      }
    }
  }

  for (i = 0; i < bestlen; i++)
    tour[i] = tour[bestind+i];
  *tourlenP = bestlen;

  delete[] seen;
}

/**
 * Solves the Traveling Salesman Problem
 *
 * @param n Number of nodes
 * @param C Vector of costs for upper triangle (size = n*(n-1)/2)
 *          Order: (0,1), (0,2), ..., (0,n-1), (1,2), (1,3), ..., (n-2,n-1)
 * @param verbosity Verbosity level:
 *          0 --> Completely silent
 *          1 --> Only custom printing
 *          2 --> Everything
 * @return TSPSolution containing tour edges and total cost
 */
TSPSolution solve_tsp(int n, const vector<double>& C, int verbosity) {

    TSPSolution result;
    result.success = false;
    result.tour_value = 0.0;

    int expected_size = n * (n - 1) / 2;
    if (C.size() != expected_size) {
        cerr << "Error: Expected " << expected_size << " cost values, got " << C.size() << endl;
        return result;
    }

    GRBEnv *env = NULL;
    GRBVar **vars = NULL;

    vars = new GRBVar*[n];
    for (int i = 0; i < n; i++)
        vars[i] = new GRBVar[n];

    try {
        //  Create empty environment**
        env = new GRBEnv(true);

        //  Set output flag on environment before starting**
        if (verbosity < 2) {
            env->set(GRB_IntParam_OutputFlag, 0);
        }

        //  Start environment (now silently)**
        env->start();

        GRBModel model = GRBModel(*env);

        model.set(GRB_IntParam_LazyConstraints, 1);

        //  Create ALL variables including diagonal
        int cont = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) {
                    // Create diagonal variable with cost 0 and upper bound 0
                    vars[i][j] = model.addVar(0.0, 0.0, 0.0, GRB_BINARY, "x_"+itos(i)+"_"+itos(j));
                } else if (i < j) {
                    // Upper triangle: use cost from C
                    vars[i][j] = model.addVar(0.0, 1.0, C[cont], GRB_BINARY, "x_"+itos(i)+"_"+itos(j));
                    cont++;
                } else {
                    // Lower triangle: point to corresponding upper triangle variable
                    vars[i][j] = vars[j][i];
                }
            }
        }

        // Degree-2 constraints: each node has exactly 2 incident edges
        for (int i = 0; i < n; i++) {
            GRBLinExpr expr = 0;
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    expr += vars[i][j];
                }
            }
            model.addConstr(expr == 2, "deg2_"+itos(i));
        }

        // Set callback
        subtourelim cb(vars, n);
        model.setCallback(&cb);

        // Optimize
        model.optimize();

        // Extract solution
        if (model.get(GRB_IntAttr_SolCount) > 0) {
            // Get solution values
            double **sol = new double*[n];
            for (int i = 0; i < n; i++) {
                sol[i] = new double[n];
                for (int j = 0; j < n; j++) {
                    if (i == j) {
                        sol[i][j] = 0.0;  // Diagonal is always 0
                    } else {
                        sol[i][j] = vars[i][j].get(GRB_DoubleAttr_X);
                    }
                }
            }

            int* tour = new int[n];
            int len;

            findsubtour(n, sol, &len, tour);

            // Check if valid tour
            if (len != n) {
                cerr << "ERROR: Found subtour of length " << len << " instead of " << n << endl;
                cerr << "Cost vector: ";
                for (size_t e = 0; e < C.size(); e++) {
                    cerr << C[e] << ",";
                }
                cerr << endl;

                // Still return the partial result for debugging
                result.success = false;
                result.tour_value = model.get(GRB_DoubleAttr_ObjVal);
            } else {
                // Extract edges from the tour
                for (int i = 0; i < len; i++) {
                    int from = tour[i];
                    int to = tour[(i + 1) % len];
                    result.tour_edges.push_back(make_pair(min(from, to), max(from, to)));
                }

                // Get objective value
                result.tour_value = model.get(GRB_DoubleAttr_ObjVal);
                result.success = true;

                // Print results
                if (verbosity >= 1) {
                    cout << "==================================" << endl;
                    cout << "TSP Solution Found" << endl;
                    cout << "==================================" << endl;
                    cout << "Tour value: " << result.tour_value << endl;
                    cout << "Tour sequence: ";
                    for (int i = 0; i < len; i++)
                        cout << tour[i] << " ";
                    cout << endl;

                    cout << "Tour edges: ";
                    for (const auto& edge : result.tour_edges) {
                        cout << "(" << edge.first << "," << edge.second << ") ";
                    }
                    cout << endl;
                    cout << "==================================" << endl;
                }
            }

            // Cleanup
            for (int i = 0; i < n; i++)
                delete[] sol[i];
            delete[] sol;
            delete[] tour;
        } else {
            cerr << "No solution found!" << endl;
        }

    } catch (GRBException e) {
        cerr << "Gurobi Error " << e.getErrorCode() << ": " << e.getMessage() << endl;
    } catch (...) {
        cerr << "Error during optimization" << endl;
    }

    // Cleanup
    for (int i = 0; i < n; i++)
        delete[] vars[i];
    delete[] vars;
    delete env;

    return result;
}

/**
 ************************* GraphTSP  *************************
 **/

// struct to hold the Graphic TSP solution
struct GraphTSPSolution {
    vector<pair<pair<int, int>, int>> walk_edges;
    vector<int> node_multiplicities;
    double tour_value;
    bool success;
};

/**
 * Solves the Graph Traveling Salesman Problem
 *
 * @param n Number of nodes
 * @param C Vector of costs for upper triangle (size = n*(n-1)/2)
 *          Order: (0,1), (0,2), ..., (0,n-1), (1,2), (1,3), ..., (n-2,n-1)
 * @param verbosity Verbosity level:
 *          0 = silent
 *          1 = basic output, custom printing
 *          2 = full Gurobi output
 * @return GraphTSPSolution containing walk edges with multiplicities and total cost
 */
GraphTSPSolution solve_graph_tsp(
    int n,
    const map<pair<int,int>, double>& edge_costs,
    int verbosity
) {
    GraphTSPSolution result;
    result.success = false;
    result.tour_value = 0.0;

    if (edge_costs.empty()) {
        cerr << "Error: No edges provided" << endl;
        return result;
    }

    try {
        // Create environment
        GRBEnv env = GRBEnv(true);
        if (verbosity < 2) {
            env.set(GRB_IntParam_OutputFlag, 0);
            env.set(GRB_IntParam_LogToConsole, 0);
        }
        env.start();

        // Create model
        GRBModel model = GRBModel(env);

        // Get list of edges
        vector<pair<int,int>> edge_list;
        for (const auto& [edge, cost] : edge_costs) {
            edge_list.push_back(edge);
        }

        // Variables x[e] for each edge (integer, 0 to 2)
        map<pair<int,int>, GRBVar> x;
        for (const auto& e : edge_list) {
            double cost = edge_costs.at(e);
            x[e] = model.addVar(0.0, 2.0, cost, GRB_INTEGER,
                               "x_" + to_string(e.first) + "_" + to_string(e.second));
        }

        // Variables d[i] for each node (integer, >= 1)
        map<int, GRBVar> d;
        for (int i = 0; i < n; i++) {
            d[i] = model.addVar(1.0, GRB_INFINITY, 0.0, GRB_INTEGER, "d_" + to_string(i));
        }

        // Degree constraints: sum(x[e] for e in delta({v})) == 2 * d[v]
        for (int v = 0; v < n; v++) {
            GRBLinExpr expr = 0;
            vector<int> v_set = {v};
            vector<pair<int,int>> incident = delta(v_set, edge_list);

            for (const auto& e : incident) {
                expr += x[e];
            }

            model.addConstr(expr == 2 * d[v], "node_deg_" + to_string(v));
        }

        // Set objective
        GRBLinExpr obj = 0;
        for (const auto& e : edge_list) {
            obj += edge_costs.at(e) * x[e];
        }
        model.setObjective(obj, GRB_MINIMIZE);

        // Initial optimization
        if (verbosity >= 1) {
            cout << "Initial optimization..." << endl;
        }
        model.optimize();

        if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
            cerr << "Initial optimization failed" << endl;
            return result;
        }

        vector<vector<int>> adj(n, vector<int>(n, 0));  // Initialize with zeros

        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                pair<int,int> edge = make_pair(i, j);

                // Check if edge exists in the graph
                if (x.count(edge) > 0 && x[edge].get(GRB_DoubleAttr_X) > 1e-6) {
                    adj[i][j] = 1;
                    adj[j][i] = 1;  // Symmetric
                }
            }
        }

        // Get connected components
        vector<vector<int>> components = getComponents(adj);

        if (verbosity >= 1) {
            cout << "  Now " << components.size() << " component(s)" << endl;
            cout << "x at this stage" << endl;
            for (auto& e : edge_list) {
                cout << e.first << " " << e.second << " " << x[e].get(GRB_DoubleAttr_X) << endl;
            }
            cout << "Adj at this stage:" << endl;
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    cout << adj[i][j] << " ";
                }
                cout << endl;
            }
            for (int i = 0; i < components.size(); i++) {
                vector<int> S_int = components[i];
                for (int j = 0; j < S_int.size(); j++) {
                    cout << S_int[j] << " ";
                }
                cout << endl;
            }

        }

        // Iteratively add subtour elimination constraints
        int iter = 0;
        while (components.size() > 1) {
            iter++;

            if (verbosity >= 1) {
                cout << "Iteration = " << iter << endl;
            }

            // Add a constraint for EVERY S
            for (int i = 0; i < components.size(); i++) {
                vector<int> S = components[i];
                GRBLinExpr expr = 0;
                vector<pair<int,int>> cut_edges = delta(S, edge_list);

                for (const auto& e : cut_edges) {
                    expr += x[e];
                }

                model.addConstr(expr >= 2, "sub_el_" + to_string(iter) +  "_" + to_string(i));
            }

            // Add constraint: sum(x[e] for e in delta(S)) >= 2


            // Re-optimize
            model.optimize();

            if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
                cerr << "Optimization failed at iteration " << iter << endl;
                return result;
            }

            adj.assign(n, vector<int>(n, 0));  // Reset to all zeros

            for (int i = 0; i < n; i++) {
                for (int j = i + 1; j < n; j++) {
                    pair<int,int> edge = make_pair(i, j);

                    // Check if edge exists in the graph
                    if (x.count(edge) > 0 && x[edge].get(GRB_DoubleAttr_X) > 1e-6) {
                        adj[i][j] = 1;
                        adj[j][i] = 1;  // Symmetric
                    }
                }
            }

            // Recompute components
            components = getComponents(adj);

            if (verbosity >= 1) {
                cout << "  Now " << components.size() << " component(s)" << endl;
                cout << "x at this stage" << endl;
                for (auto& e : edge_list) {
                    cout << e.first << " " << e.second << " " << x[e].get(GRB_DoubleAttr_X) << endl;
                }
                cout << "Adj at this stage:" << endl;
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        cout << adj[i][j] << " ";
                    }
                    cout << endl;
                }
                for (int i = 0; i < components.size(); i++) {
                    vector<int> S_int = components[i];
                    for (int j = 0; j < S_int.size(); j++) {
                        cout << S_int[j] << " ";
                    }
                    cout << endl;
                }
            }
        }

        // Extract final solution
        result.success = true;
        result.tour_value = model.get(GRB_DoubleAttr_ObjVal);

        // Get node multiplicities
        result.node_multiplicities.resize(n);
        for (int i = 0; i < n; i++) {
            result.node_multiplicities[i] = static_cast<int>(d[i].get(GRB_DoubleAttr_X));
        }

        // Get edges with multiplicities
        for (const auto& e : edge_list) {
            double value = x[e].get(GRB_DoubleAttr_X);
            if (value > 0.5) {  // Changed: only include if used
                result.walk_edges.push_back({e, static_cast<int>(value)});
            }
        }

        if (verbosity >= 1) {
            cout << "\n==================================\n";
            cout << "Graph TSP Solution Found\n";
            cout << "==================================\n";
            cout << "Tour value: " << result.tour_value << endl;
            cout << "==================================\n";
        }

    } catch (GRBException& e) {
        cerr << "Gurobi Error " << e.getErrorCode() << ": " << e.getMessage() << endl;
    } catch (...) {
        cerr << "Unknown error during optimization" << endl;
    }

    return result;
}



