#pragma once
#include <vector>
#include <string>
#include <map>
#include <chrono>

using namespace std;

/**
 * @brief Computes the connected components of an undirected graph.
 *
 * @param adj Adjacency list of the graph.
 * @return A vector containing the connected components. Each component is
 *         represented as a vector of nodes indices.
 */
vector<std::vector<int>> getComponents(const vector<vector<int>>& adj);

/**
 * @brief Converts an undirected edge {i,j} into its index in an edge vector sorted lexicographically.
 *
 *
 * @param i First endpoint of the edge.
 * @param j Second endpoint of the edge.
 * @param n Number of vertices.
 * @return The index associated with edge {i,j}.
 */
int from_i_j_to_e(int i, int j, int n);

/**
 * @brief Converts an integer to a string.
 *
 * @param i Integer to convert.
 * @return The corresponding string representation.
 */
string itos(int i);

/**
 * @brief Computes the cut-set δ(S).
 *
 * @param S Set of nodes defining the cut.
 * @param edges List of edges of the graph.
 * @return The set of edges crossing the cut defined by S.
 */
vector<pair<int,int>> delta(
    const vector<int>& S,
    const vector<pair<int,int>>& edges
);

/**
 * @brief Representation of a vertex of P_{\text{SEP}^n}
 */
using Vertex = map<pair<int, int>, double>;

/**
 * @brief Representation of edge costs.
 *
 * The map associates each edge with its corresponding cost.
 */
using Cost = map<pair<int, int>, double>;

/**
 * @brief Representation of an undirected edge.
 */
using Edge = pair<int, int>;

/**
 * @brief Representation of a walk.
 *
 * The map associates each directed edge with the number of times it is
 * traversed by the walk.
 */
using Walk = map<pair<int, int>, int>;

/**
 * @brief Performs what is called in the paper BB-Move, that is, insert in a vertex a node in a 1-path
 *
 * Given a fractional solution x0 and an edge e, performs the corresponding
 * branching/move operation and returns the resulting solution.
 *
 * @param n Number of nodes.
 * @param x0 Input fractional solution.
 * @param e Edge on which the move is performed.
 * @param tol Numerical tolerance used to determine whether a value is
 *            considered integral. Default is 1e-6.
 * @return The resulting fractional vertex with 1-path expanded.
 */
Vertex bbmove(int n, const Vertex& x0, Edge e, double tol = 1e-6);

/**
 * @brief Displays the progress of a computation.
 *
 * @param current Number of items processed so far.
 * @param total Total number of items to process.
 * @param start Time at which the computation started.
 * @param bar_width Width of the progress bar in characters.
 *
 */
void print_progress_bar(int current, int total,
                        chrono::time_point<chrono::high_resolution_clock> start,
                        int bar_width = 50);

/**
 * @brief Extends a walk after applying a BB-move to a 1-edge.
 *
 *
 * @param n Index of the new node introduced by the extension, as well as
 *          the number of nodes
 * @param w Input walk to be extended.
 * @param e 1-edge on which the extension is performed.
 *
 * @return A vector containing the walk(s) obtained after the extension.
 */
vector<Walk> extend_walk(int n, const Walk& w, Edge e);

/**
 * @brief Generates all subsets of a set with a prescribed cardinality.
 *
 * Generates all subsets of the set \f$\{0,\ldots,n-1\}\f$ containing exactly
 * @p cardinality elements.
 *
 * @param n Number of elements in the ground set.
 * @param cardinality Required cardinality of each subset.
 *
 * @return A vector containing all subsets of \f$\{0,\ldots,n-1\}\f$ with
 *         exactly @p cardinality elements.
*/
vector<vector<int>> get_subsets_by_cardinality(int n, int cardinality);



