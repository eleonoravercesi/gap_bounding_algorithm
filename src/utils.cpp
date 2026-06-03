#include <bits/stdc++.h>
using namespace std;
#include <vector>
#include "utils.h"
#include "GB_algorithm.h"

std::vector<std::vector<int>> getComponents(
    const std::vector<std::vector<int>>& adj)
{
    int n = adj.size();
    std::vector<bool> visited(n, false);
    std::vector<std::vector<int>> components;

    for (int start = 0; start < n; ++start)
    {
        if (visited[start]) continue;

        std::vector<int> comp;
        std::stack<int> st;
        st.push(start);
        visited[start] = true;

        while (!st.empty())
        {
            int u = st.top(); st.pop();
            comp.push_back(u);

            for (int i = 0; i < n; i++) //Could be here
            { int v = adj[u][i];
                if (v == 1 && !visited[i])
                {
                    visited[i] = true;
                    st.push(i);
                }
            }
        }

        components.push_back(comp);
    }

    return components;
}

vector<int> oneComponent(vector<vector<int>> adj) {
    vector<int> S = {0};

    // Copy adj to avoid modifying the original
    vector<vector<int>> adj_copy = adj;

    // Safeguard: prevent infinite loop
    int max_iterations = adj.size() + 10;
    int iterations = 0;

    while (iterations < max_iterations) {
        iterations++;
        int last_of_S = S.back();

        // If we closed the tour back to start, we're done
        if (S.size() > 1 && last_of_S == S[0]) {
            S.pop_back();  // remove duplicate start node
            return S;
        }

        // Find an edge incident to the last node in S
        bool found = false;
        for (int e = 0; e < (int)adj_copy.size(); ++e) {
            if (last_of_S == adj_copy[e][0]) {
                S.push_back(adj_copy[e][1]);
                adj_copy.erase(adj_copy.begin() + e);
                found = true;
                break;
            }
            if (last_of_S == adj_copy[e][1]) {
                S.push_back(adj_copy[e][0]);
                adj_copy.erase(adj_copy.begin() + e);
                found = true;
                break;
            }
        }

        // No edge found means the component containing 0 is exhausted
        // i.e. we have a subtour that doesn't span all nodes
        if (!found) {
            return S;  // return the partial component as-is
        }
    }

    cerr << "Warning: oneComponent exceeded max iterations, returning partial component\n";
    return S;
}

std::vector<int> edges_to_tour(const std::vector<std::pair<int,int>>& edges, int n) {
    // Build adjacency list
    vector<int> tour = {0};

    // Make a working copy of edges (there should be n edges in a tour)
    vector<pair<int,int>> edges_copy = edges;

    int current_node = tour.back();
    int cont = 1;
    while (cont < n + 1) {
        for (int e = 0; e < edges_copy.size(); ++e) {
            if (edges_copy[e].first == current_node) {
                tour.push_back(edges_copy[e].second);
                current_node = edges_copy[e].second;

                // Erase from edge
                edges_copy.erase(edges_copy.begin() + e);
                cont++;
                break;
            }
            if (edges_copy[e].second == current_node) {
                tour.push_back(edges_copy[e].first);
                current_node = edges_copy[e].first;

                // Erase from edge
                edges_copy.erase(edges_copy.begin() + e);
                cont++;
                break;
            }
        }
    }
    return tour;
}

int from_i_j_to_e(int i, int j, int n) {
    int e;
    e = (n - 2) * i - (i - 1) * (i ) / 2 + j - 1;
    return e;
}

string itos(int i) {stringstream s; s << i; return s.str(); }


vector<pair<int,int>> delta(
    const vector<int>& S,
    const vector<pair<int,int>>& edges
) {
    vector<pair<int,int>> result;

    // Get the number of edges here
    int n_edges = edges.size();
    int e, i, j;

    for (e = 0; e < n_edges; ++e) {
        i = edges[e].first;
        j = edges[e].second;

        int i_in_S = count(S.begin(), S.end(), i);
        int j_in_S = count(S.begin(), S.end(), j);

        if (i_in_S + j_in_S == 1) {
            result.push_back(make_pair(i, j));
        }
    }

    return result;
}

/*
 * @param n Number of nodes (the new node will be node n)
 * @param x0 Vertex of the SEP polytope
 * @param e The 1-edge to branch on
 * @param tol Tolerance for checking if x0[e] == 1
 * @return a Vertex "expanded"
 */

using Vertex = map<pair<int, int>, double>;
using Edge = pair<int, int>;

Vertex bbmove(int n, const Vertex& x0, Edge e, double tol) {
    Vertex x1;

    // Check that x0[e] == 1 (within tolerance)
    if (abs(x0.at(e) - 1.0) >= tol) {
        cout << "Edge e is not a one-edge (x0[e] = " +
                               to_string(x0.at(e)) + ", expected 1.0). Returning empty vertex";
        return x1;
    }

    // Otherwise, Create a copy of x0
    x1 = x0;

    // For all nodes v in {0, 1, ..., n-1}, set x1[(v, n)] = 0
    for (int v = 0; v < n; v++) {
        Edge edge_to_n = make_pair(min(v, n), max(v, n));
        x1[edge_to_n] = 0.0;
    }

    // Set x1[e] = 0
    x1[e] = 0.0;

    // Set x1[(e[0], n)] = 1 and x1[(e[1], n)] = 1
    Edge edge1 = make_pair(min(e.first, n), max(e.first, n));
    Edge edge2 = make_pair(min(e.second, n), max(e.second, n));

    x1[edge1] = 1.0;
    x1[edge2] = 1.0;

    return x1;
}

/**
 * Returns the walk(s) obtained extending the walk w
 * after one application of the bbmove on the 1-edge e.
 */
vector<Walk> extend_walk(int n, const Walk& w, Edge e) {
    vector<Walk> extended_walks;

    // Get multiplicity of edge e in walk w
    int mult = 0;
    if (w.count(e) > 0) {
        mult = w.at(e);
    }

    // Create edges to new node n
    Edge edge_e0_n = make_pair(min(e.first, n), max(e.first, n));
    Edge edge_e1_n = make_pair(min(e.second, n), max(e.second, n));

    if (mult == 0) {
        // Case 1: Edge e is not used in walk
        Walk w1 = w;
        w1.erase(e);
        w1[edge_e0_n] = 2;
        if (w1[edge_e1_n] == 0) {
            w1.erase(edge_e1_n);  // Clean up zero
        }

        Walk w2 = w;
        w2.erase(e);
        if (w2[edge_e0_n] == 0) {
            w2.erase(edge_e0_n);  // Clean up zero
        }
        w2[edge_e1_n] = 2;

        extended_walks.push_back(w1);
        extended_walks.push_back(w2);

    } else {
        // Case 2: Edge e is used in walk
        Walk w1 = w;
        w1.erase(e);
        w1[edge_e0_n] = mult;
        w1[edge_e1_n] = mult;

        extended_walks.push_back(w1);
    }

    return extended_walks;
}

