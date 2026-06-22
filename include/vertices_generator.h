// Header guard to prevent multiple inclusion / circular-include redefinition
#ifndef VERTICES_GENERATOR_H
#define VERTICES_GENERATOR_H

#include <vector>
#include "gmp.h"
#include "nauty_wrapper.h"
#include "ppl.hh"
#include <format>
#include <set>
#include <algorithm>
#include <cmath>
using namespace std;

// Define a new type for vertices in fractions
// Structure to hold TSP solution results
struct VertexFraction {
    int den;  // Denominator
    vector<int> v; // Numerator

    // Default constructor
    VertexFraction() : den(1) {}

    // Constructor from GMP types
    VertexFraction(const GMP_Integer& denominator, const vector<GMP_Integer>& numerators) {
        // Convert GMP_Integer denominator to int
        den = (int)denominator.get_si();  // get_si() converts to signed int

        // Convert vector of GMP_Integer to vector of int
        for (const auto& num : numerators) {
            v.push_back((int)num.get_si());
        }
    }

    // Get n
    int getn() {
        return (1  + sqrt(1 + 8 * v.size())) / 2 ;
    }

    // Get numerators
    vector<int> getNumerators() {
        return v;
    }

    string toString(int n) {
        stringstream ss;
        ss << format("{},", n);

        for (size_t i = 0; i < v.size(); ++i) {
            ss << format("{}/{}", v[i], den);
            if (i == v.size() - 1) {
                ss << "\n";
            }
            else {
                ss << " ";
            }
        }
        return ss.str();
    }

    // Is int? If den = 1
    bool isInt() {
        return den == 1;
    }


    // Number of non-zeros components
    int numberOfNonZeros(int n) {
        // It is easy to count the zeros
        int number_of_zeros = count(v.begin(), v.end(), 0);
        int m = n * ( n  - 1 ) / 2;
        return m - number_of_zeros;
    }

    // // Convert it to coloured graph
    // graph* toGraph(int n, int &out_nd) {
    //     // TODO this function must be verified another time at least
    //     // Take vector<int> v
    //     // Count all the different values -- excluding zero
    //     std::set<int> unique_values; // I need the one of std, not the one of nauty
    //
    //     for (int val : v) {
    //         if (val != 0) {  // Exclude zero
    //             unique_values.insert(val);
    //         }
    //     }
    //
    //     // Sort unique values
    //     vector<int> unique_values_sorted(unique_values.begin(), unique_values.end());
    //
    //     sort(unique_values_sorted.begin(), unique_values_sorted.end());
    //
    //     // Different non zeros values
    //     int diff_values = unique_values.size();
    //
    //     int d;
    //
    //     // Get the value d such that diff_values <= 2**d - 1
    //     d = ceil(log2(diff_values + 1));
    //
    //     // Make amn adjacency matrix
    //     vector<vector<int>> adj(n * d, vector<int>(n * d, 0));
    //
    //     // Note: the formula i d' x n + n'
    //
    //     // Step 1: all the nodes with the same n' but different d are connectet
    //     for (int n_prime = 0; n_prime < n; n_prime++) {
    //         for (int d_prime = 0; d_prime < d - 1; d_prime++) {
    //             int a = d_prime * n + n_prime;
    //             int b = (d_prime + 1) * n + n_prime;
    //
    //             adj[a][b] = 1;
    //             adj[b][a] = 1;
    //         }
    //     }
    //
    //     // Step 2: each layer is for just one symbol, apart from the last one, that is in all of them
    //     int e = 0;
    //     for (int i = 0; i < n; i++) {
    //         for (int j = i + 1; j < n; j++) {
    //             // Get the numerator
    //             int v_e = v[e];
    //             // Increase e
    //             e++;
    //             if (v_e != 0) {
    //                 // Then there is an edge! with which weight? I need the index in the vector unique_values
    //                 auto it = find(unique_values_sorted.begin(), unique_values_sorted.end(), v_e) - unique_values_sorted.begin();
    //
    //                 // Increase it by 1: given the doc, we want numbers from 1 to 2**d - 1
    //                 it = it + 1;
    //
    //
    //                 for (int d_prime = 0; d_prime < d; ++d_prime) {
    //                     bool x_d_prime = (it >> d_prime) & 1;  // Extract bit i from it
    //
    //                     adj[d_prime * n + i][d_prime * n + j] = x_d_prime;
    //                     adj[d_prime * n + j][d_prime * n + i] = x_d_prime;
    //                 }
    //
    //             }
    //         }
    //     }
    //
    //     // From here: claude did it
    //     int nd = n * d;  // Total number of nodes in the nauty graph
    //     out_nd = nd;     // Return nd to the caller
    //
    //     // Convert adjacency matrix to nauty graph of size nd
    //     int m = (nd + WORDSIZE - 1) / WORDSIZE;
    //     graph* g = (graph*)malloc(nd * m * sizeof(graph));
    //     if (!g) return nullptr;
    //
    //     for (int i = 0; i < nd * m; ++i) g[i] = 0;
    //
    //     for (int i = 0; i < nd; ++i) {
    //         for (int j = 0; j < nd; ++j) {
    //             if (adj[i][j] != 0) {
    //                 ADDELEMENT(&g[i * m], j);
    //             }
    //         }
    //     }
    //
    //     return g;
    //
    // }

    map<pair<int,int>, int> toCounter() {
        vector<int> v_sorted = v;
        sort(v_sorted.begin(), v_sorted.end());

        map<pair<int,int>, int> counter;
        for (int num : v_sorted) {
            counter[{num, den}]++;  // Exact rational, no floating point
        }
        return counter;
    }

    bool isMinimal() {
        for (int e = 0; e < v.size(); ++e) {
            int num = v[e];

            // If we find a "minimal" element, we can return true immediately
            if (num > 0 && num % den != 0){ // It means that you cannot simplify num and den, so is minimal!
                return true;
            }
        }
        // Only return false if we checked the WHOLE loop and never hit the 'true' condition
        return false;
    }

    void Minimalize() {
        if (not isMinimal()) {
            for (int e = 0; e < v.size(); ++e) {
                v[e] = v[e] / den; // Integer division, hence safe
            }
        }
    }

    vector<vector<int>> toMatrix(int n) {
        // Initialize n x n matrix with zeros
        vector<vector<int>> X(n, vector<int>(n, 0));

        int e = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {  // Upper triangle only: j starts at i+1
                X[i][j] = v[e];
                X[j][i] = v[e];  // Symmetric
                e++;
            }
        }

        return X;
    }

};


// Declaration of function provided by the implementation
vector<VertexFraction> get_all_vertices_with_graph_constraints(int n, int k, vector<int> G, bool with_int);

#endif // VERTICES_GENERATOR_H

