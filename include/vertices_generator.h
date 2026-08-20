// Header guard to prevent multiple inclusion / circular-include redefinition
#ifndef VERTICES_GENERATOR_H
#define VERTICES_GENERATOR_H

#include <vector>
#include "gmp.h"
#include "ppl.hh"
#include <format>
#include <algorithm>
#include <cmath>
using namespace std;

/**
 * @brief Represents a vertex using exact rational values.
 *
 * The vertex is represented as a vector of numerators with a common
 * denominator. The value of component i is given by v[i] / den.
 *
 * For a complete graph on n vertices, the vector v contains
 * n(n-1)/2 components, one for each edge.
 */
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

    /**
     * @brief Returns the number of vertices of the underlying complete graph.
     *
     * @return The number of vertices n.
     */
    int getn() {
        return (1  + sqrt(1 + 8 * v.size())) / 2 ;
    }


    /**
     * @brief Get the numerators as vector of integers
     * @return Returns the numerators
     */
    vector<int> getNumerators() {
        return v;
    }

    /**
     * @brief This method does a nice printing of a vertex
     *
     * @param n, int, number of nodes
     * @return The vertex as a string
     */
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


    /**
     * @brief Is this vertex integer?
     *
     * @return
     */
    bool isInt() {
        return den == 1;
    }


    /**
     * @brief Count the number of non zeros
     *
     * @param n , integer, number of nodes of the TSP
     *
     * @return  The number of non zeros as an integer
     */
    int numberOfNonZeros(int n) {
        // It is easy to count the zeros
        int number_of_zeros = count(v.begin(), v.end(), 0);
        int m = n * ( n  - 1 ) / 2;
        return m - number_of_zeros;
    }

    /**
     * @brief Computer a Counter of type (num, den) --> counter counting how many times a fraction appear in a vertex
     *
     * @return The counter in question
     */
    map<pair<int,int>, int> toCounter() {
        vector<int> v_sorted = v;
        sort(v_sorted.begin(), v_sorted.end());

        map<pair<int,int>, int> counter;
        for (int num : v_sorted) {
            counter[{num, den}]++;  // Exact rational, no floating point
        }
        return counter;
    }


    /**
     * @brief Convert the vertex x to a symmetric matrix that contains the NUMERATORS only
     *
     * @param n Number of nodes of the TSP
     *
     * @return The matrix in question
     */
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


/**
 * @brief Given a graph G, returns all the vertices of $P_{\text{SEP}^n } having the support graph contained in G and precisely n + k edges
 *
 * @param n An integer, with the dimension of the TSP
 * @param k Integer, the k in question
 * @param G Graph, the graph
 * @param with_int If true, we also return integer vertices
 *
 * @return A Vector of Vertex Fraction
 */
vector<VertexFraction> get_all_vertices_with_graph_constraints(int n, int k, vector<int> G, bool with_int);

#endif // VERTICES_GENERATOR_H

