#include "vertices_generator.h"
#include "ppl.hh"
using namespace std;
using namespace Parma_Polyhedra_Library;
#include "GB_algorithm.h" // For the Edge class
#include "utils.h"
#include <fstream>
#include <gmp.h>


VertexFraction get_generator(const C_Polyhedron& poly, const Generator& gen) {
    auto den = gen.divisor();
    vector<GMP_Integer> num;

    for (size_t i = 0; i < poly.space_dimension(); ++i) {
        Coefficient coeff = gen.coefficient(Variable(i));
        num.push_back(coeff);
    }

    VertexFraction vertex(den, num);
    return vertex;

}

vector<VertexFraction> get_all_vertices_with_graph_constraints(int n, int k, vector<int> G, bool with_int) {
    // Define m
    int m = n * (n - 1) / 2;

    vector<VertexFraction> vertices_this_graph;

    C_Polyhedron poly(m); // This should be the polytope


    // Create a map (i, j) --> e
    vector<Edge> edges(m);
    map<Edge, int> edges_dict;
    int i, j;
    int e = 0;
    for (i = 0; i < n; ++i) {
        for (j = i + 1; j < n; ++j) {
            edges_dict[make_pair(i, j)] = e;
            edges[e] = make_pair(i, j);
            e++;
        }
    }

    assert(e == m );

    // Add all the variable non neg and leq 1
    for (e = 0; e < m; e ++) {
        poly.add_constraint(Variable(e) >= 0);
        poly.add_constraint(Variable(e) <= 1);
    }

    // Add degree constraints
    for (i = 0; i < n; ++i) {
        Linear_Expression expr;
        vector<int> S = {i};
        vector<Edge> out = delta(S, edges);
        for (e = 0; e < out.size(); ++e) {
            expr += Variable(edges_dict[out[e]]);
        }
        poly.add_constraint(expr == 2);
    }

    // Add subtour elimination constraints only for sets having a cardinality <= ceil n / 2
    int n_prime, s;
    int n_max = ceil(n / 2 );
    for (n_prime = 3; n_prime <= n_max ; n_prime++) {
        vector<vector<int>> S_list = get_subsets_by_cardinality(n, n_prime);

        // Now add the constraints!
        for (s = 0; s < S_list.size(); s++) {
            Linear_Expression expr;
            vector<int> S = S_list[s];
            vector<Edge> out = delta(S, edges);
            for (e = 0; e < out.size(); ++e) {
                expr += Variable(edges_dict[out[e]]);
            }
            poly.add_constraint(expr >= 2);
        }
    }

    // Add the constraint on graph G
    for (e = 0; e < m; e ++) {
        if (G[e] == 0) {
            poly.add_constraint(Variable(e) == 0);
        }
    }

    // Extract vertices
    for (auto gen : poly.generators()) {
        if (gen.is_point()) {
            VertexFraction this_vertex = get_generator(poly, gen);
            // If I want integers, should be integer
            if (with_int or not this_vertex.isInt()) {
                if (this_vertex.numberOfNonZeros(n) == n + k) {
                    vertices_this_graph.push_back(this_vertex);
                }
            }
        }
    }
    return vertices_this_graph;
}