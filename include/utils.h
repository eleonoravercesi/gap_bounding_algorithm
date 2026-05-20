#pragma once
#include <vector>

using namespace std;

vector<int> oneComponent(vector<vector<int>> adj);
vector<std::vector<int>> getComponents(const vector<vector<int>>& adj);
vector<int> edges_to_tour(const std::vector<std::pair<int,int>>& edges, int n);

