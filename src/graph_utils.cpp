#include "graph_utils.h"

#include <functional>
#include <ranges>

using std::function, std::map, std::pair, std::vector;


namespace {
    int find(vector<int>& parent, int i) {
        if (parent[i] != i)
            parent[i] = find(parent, parent[i]);

        return parent[i];
    }


    void join(vector<int>& parent, int i, int j) {
        int pi = find(parent, i);
        int pj = find(parent,j);
        if (pi != pj)
            parent[pi] = parent[pj];
    }
}


vector<vector<int>> connected_components(int n, const vector<Edge>& edges) {
    vector<int> parent(n);
    for (int i = 0; i < n; ++i)
        parent[i] = i;
    
    for (auto& [i, j] : edges)
        join(parent, i, j);
    
    map<int, vector<int>> parent_map;
    for (int i = 0; i < n; ++i)
        parent_map[find(parent, i)].push_back(i);
    
    vector<vector<int>> components;
    for (auto &nodes: parent_map | views::values)
        components.push_back(std::move(nodes));
    
    return components;
}