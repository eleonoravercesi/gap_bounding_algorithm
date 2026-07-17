#pragma once
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <chrono>

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
using Cost = map<pair<int, int>, double>;
using Edge = pair<int, int>;
using Walk = std::map<std::pair<int, int>, int>;

Vertex bbmove(int n, const Vertex& x0, Edge e, double tol = 1e-6);

vector<Walk> extend_walk(int n, const Walk& w, Edge e);

// Generate all subsets of a given set with specific cardinality
vector<vector<int>> get_subsets_by_cardinality(int n, int cardinality);

// Print vector of vectors of int
void print_vector_of_vector_of_int(vector<vector<int>>& v);

void print_progress_bar(int current, int total,
                        chrono::time_point<chrono::high_resolution_clock> start,
                        int bar_width = 50);
