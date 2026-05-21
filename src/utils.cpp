#include <bits/stdc++.h>
using namespace std;
#include <vector>
using namespace std;

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

            for (int v : adj[u])
            {
                if (!visited[v])
                {
                    visited[v] = true;
                    st.push(v);
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
