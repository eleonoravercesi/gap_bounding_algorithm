/**
 * This file containts several routine; you can run whatever you wish just selecting the right flag; more specifically
 * "opt_plus" run the opt_plus algorithm on vertices of a given size and produces a file with the maximum gap
 **/


#include "nauty_wrapper.h"
#include "ppl.hh"
#include <vector>
#include "readers.h"
#include "GB_algorithm.h"
#include <format>
#include <fstream>
#include <chrono>
#include <ranges>

#include "utils.h"
#include "graphs.h"
#include "opt_hat.h"

// Move the `using namespace std;` below so that standard library
// symbols are not injected into the global namespace before
// including C headers like nauty.h (which declares an identifier
// named `set`). This avoids the ambiguous reference between
// `std::set` and nauty's `typedef setword set`.
using namespace std;

using std::chrono::duration;
using namespace Parma_Polyhedra_Library;

// TODO for the GBE check old

void get_ancestors(int k) {
    string filename_out = format("../ancestors/ancestors_new_{}.csv", k);

    // Write a header on filename
    ofstream myfile;
    myfile.open (filename_out);
    myfile << "n,xi\n";
    myfile.close();

    for (int n = k + 3; n <= 2*k; ++n) {
        string filename = format("../graphs/{}_{}.txt", n, k);
        vector<vector<int>> G_list = read_upper_triangle_graphs(filename);
        int n_graphs = G_list.size();
        //cout << "Ready to parse " << n_graphs << " graphs" << endl;

        auto start = chrono::high_resolution_clock::now();
        for (int g = 0; g < n_graphs; ++g) {
            print_progress_bar(g, n_graphs, start);
            // auto t1 = std::chrono::high_resolution_clock::now();
            vector<int> current_graph = G_list[g];
            vector<VertexFraction> vertices_for_this_g =  get_all_vertices_with_graph_constraints(n, k, current_graph, false);

            int n_vertices_for_this_g = vertices_for_this_g.size();
            //cout << "\t Vertices originated from this graph: " << n_vertices_for_this_g << endl;
            if (n_vertices_for_this_g > 0) {
                vector<int> unique_idx = filter_isomorphic_graphs(vertices_for_this_g, n,  0);

                // // Dummy placeholder
                // vector<int> unique_idx;
                // for (int u = 0; u < n_vertices_for_this_g; ++u) {
                //     unique_idx.push_back(u);
                // }

                //cout << "\t\t After filtering... " << unique_idx.size() << endl;
                // Open file in append mode (creates if doesn't exist, appends if exists)
                ofstream outfile(filename_out, ios::app);

                if (!outfile.is_open()) {
                    cerr << "Error: Cannot open or create file " << filename << "\n";
                    exit(1);
                }

                for (int u = 0; u < unique_idx.size();  ++u) {
                    outfile << vertices_for_this_g[unique_idx[u]].toString(n);
                }
                outfile.close();
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            //cout << "\t" << g + 1 << "/" << n_graphs << " done in " << (t2 - t1) * 1e-9 << " seconds." << endl;
        }
    }
}

void gb_test(string k) {
    // Here, k is from 3 onward!!

    //string filename = format("/home/vercee/Documents/math_prog_extended/ancestors/ancestors_new_{}.csv", k);
    string filename = format("/home/vercee/Documents/math_prog_extended/ancestors/ancestors_new_{}_more_iter.csv", k);
    vector<VertexFraction> x_list;
    x_list = read_ancestors(filename);

    // Open a fine to write idx,opt_plus,gap_plus,runtime
    ofstream fout;
    fout.open(format("/home/vercee/Documents/math_prog_extended/output/results_GBe_{}.csv", k));

    // Write the header
    fout << "n,gapII,family_gapII,iterations,runtime\n";

    // Set a high precision so we can recover fractions
    fout << setprecision(20);

    // For loop on the vectors
    int t, i, j, e;
    auto start = chrono::high_resolution_clock::now();
    for (t = 0; t < x_list.size(); ++t) {
        print_progress_bar(t, x_list.size(), start);
        int n_int = x_list[t].getn();
        int m = x_list[t].v.size();
        double den = x_list[t].den;
        vector<double> x_0(m);

        for (int e = 0; e < m; ++e) {
         x_0[e] = (double)x_list[t].v[e] / den;
        }

        map<pair<int,int>, double> x_0_dict;

        for (i = 0; i < n_int; ++i) {
            for (j = i + 1; j < n_int; ++j) {
                e = from_i_j_to_e(i, j, n_int);
                if (x_0[e] > 0) {
                    x_0_dict[make_pair(i,j)] = x_0[e]; // [ ] for insertion, .at for access
                }
            }
        }


        auto t1 = std::chrono::high_resolution_clock::now();
        GBAlgorithmResult result;
        vector<Walk> w_start;

        double target_gap = 4.0/3.0;
        result = GBe_algorithm(n_int, x_0_dict, target_gap, 40, 1e-6, 0);
        auto t2 = std::chrono::high_resolution_clock::now();

        /* Getting number of milliseconds as a double. */
        duration<double, std::milli> ms_double = t2 - t1;

        double gap_II_val = result.gapII;
        double family_gap = result.family_gapII;
        //cout << "Done with " << t << " out of " << x_list.size() << " with family gap " << family_gap << endl;

        int iterations = result.iterations;

        // Now, write on file
        fout << n_int << "," << gap_II_val << "," << family_gap << "," << iterations << ","  << ms_double << "\n";
    }
    fout.close();
}


int main(int argc, char *argv[]) {
    // int k = 8;
    // cout << "✨ Ready for k = " << k << endl;
    // string k_str = itos(k);
    // cout << "✨ COMPUTING ANCESTORS ✨" << endl;
    // get_ancestors(k);
    // cout << endl << "✨ RUN GB ✨" << endl;
    // gb_test(k_str);


    Vertex x = tetrahedron_instance();
    GTSPSolution sol = solve_gtsp(x);

    cout << sol.opt_value << endl;
    for (Edge e : sol.opt_walk | std::views::keys) {
        cout << "(" << e.first + 1 << ", " << e.second + 1 << "): " << sol.opt_walk.at(e) << endl;
    }

    return 0;
}


