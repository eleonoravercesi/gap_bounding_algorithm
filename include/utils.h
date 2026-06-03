#pragma once
#include <vector>
#include <string>
#include <map>

using namespace std;

vector<int> oneComponent(vector<vector<int>> adj);

vector<std::vector<int>> getComponents(const vector<vector<int>>& adj);

vector<int> edges_to_tour(const std::vector<std::pair<int,int>>& edges, int n);

int from_i_j_to_e(int i, int j, int n);

string itos(int i);

vector<pair<int,int>> delta(
    const vector<int>& S,
    const vector<pair<int,int>>& edges
);

using Vertex = map<pair<int, int>, double>;
using Edge = pair<int, int>;
using Walk = std::map<std::pair<int, int>, int>;

Vertex bbmove(int n, const Vertex& x0, Edge e, double tol = 1e-6);

vector<Walk> extend_walk(int n, const Walk& w, Edge e);