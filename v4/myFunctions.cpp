#include "myFunctions.hpp"
#include <minisat/core/Solver.h>
#include <minisat/core/SolverTypes.h> 
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <utility>
#include <set>
#include <future>
#include <unordered_map>
#include <unordered_set>

using namespace std;

/*
CNF SAT reduction
*/
vector<int> cnfReduction(int n, int k, const vector<pair<int, int>>& edges, int timeout_seconds) {
    // Initialize solver
    unique_ptr<Minisat::Solver> solver(new Minisat::Solver());

    // Create n X k matrix for boolean literals
    vector<vector<Minisat::Var>> x(n, vector<Minisat::Var>(k));

    // Create x_{i,j} matrix and add them to solver
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            x[i][j] = solver->newVar();
        }
    }

    // First encoding: At least one vertex is the i-th vertex in the vertex cover
    for (int i = 0; i < k; i++) {
        Minisat::vec<Minisat::Lit> clause;
        for (int N = 0; N < n; N++) {
            clause.push(Minisat::mkLit(x[N][i]));
        }
        solver->addClause(clause);
    }

    // Second encoding: No one vertex can appear twice in vertex cover
    for (int m = 0; m < n; m++) {
        for (int p = 0; p < k; p++) {
            for (int q = p + 1; q < k; q++) {
                solver->addClause(~Minisat::mkLit(x[m][p]), ~Minisat::mkLit(x[m][q]));
            }
        }
    }

    // Third encoding: No more than one vertex appears in the mth position of the vertex cover
    for (int m = 0; m < k; m++) {
        for (int p = 0; p < n; p++) {
            for (int q = p + 1; q < n; q++) {
                solver->addClause(~Minisat::mkLit(x[p][m]), ~Minisat::mkLit(x[q][m]));
            }
        }
    }

    // Fourth encoding: Every edge is incident to at least one vertex in the vertex cover
    for (const auto& edge : edges) {
        int i = edge.first;
        int j = edge.second;
        Minisat::vec<Minisat::Lit> clause;
        for (int l = 0; l < k; l++) {
            clause.push(Minisat::mkLit(x[i - 1][l])); // i-1 since lowest vertex value is 1
            clause.push(Minisat::mkLit(x[j - 1][l])); // j-1 since lowest vertex value is 1
        }
        solver->addClause(clause);
    }

    // Solve the CNF formula now using MiniSat
    // Wait for the solver to finish or timeout
    bool sat = false;
    //Start a thread to run the solver
    future<void> solver_future = async(launch::async, [&]() {
        sat = solver->solve();
    });
    future_status status = solver_future.wait_for(chrono::seconds(timeout_seconds));
    if (status == future_status::timeout) {
    // Timeout occurred
        solver->interrupt(); // Try to interrupt the solver
        return {-1};
    }
    if (!sat) {
        // Solver did not finish in time, or no solution exists
        return {-1};
    }

    vector<int> cover;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            if (solver->modelValue(x[i][j]) == Minisat::l_True) {
                cover.push_back(i + 1);
            }
        }
    }

    return cover;
}

/*
use binary search algorithm to find the minimum vertex cover (i.e. smallest k)
*/
pair<int, vector<int>> binarySearchCoverage(int n, const vector<pair<int, int>>& edges, int timeout_seconds) {
    int low = 1;
    int high = n-1;

    while (low < high) {
        int mid = low + (high - low) / 2;
        vector<int> result = cnfReduction(n, mid, edges, timeout_seconds);
        if (result[0] == -1) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    // low is the minimum k for which a vertex cover exists
    return {low, cnfReduction(n, low, edges, timeout_seconds)};
}

/*
APPROX_VC_1
*/
pair<int, vector<int>> approxVC1(vector<pair<int, int>>& originalE) {
    std::vector<std::pair<int, int>> E = originalE; // make a copy
    vector<int> vertex_cover;

    while (!E.empty()) {
        // create a map to store the degree of each vertex
        unordered_map<int, int> degree_map;
        // calculate the degree of each vertex
        for (const auto& edge : E) {
            degree_map[edge.first]++;
            degree_map[edge.second]++;
        }
        // find the vertex with the highest degree
        int max_degree_vertex = -1;
        int max_degree = -1;

        for (const auto& entry : degree_map) {
            if (entry.second > max_degree) {
                max_degree = entry.second;
                max_degree_vertex = entry.first;
            }
        }
        // add the vertex to the vertex cover
        vertex_cover.push_back(max_degree_vertex);
        // remove edges incident on the selected vertex
        E.erase(remove_if(E.begin(), E.end(), [max_degree_vertex](const pair<int, int>& edge) {
                                   return edge.first == max_degree_vertex || edge.second == max_degree_vertex;
                                   }),
                E.end());
    }
    // sort the vertex_cover in ascending order
    sort(vertex_cover.begin(), vertex_cover.end());

    return make_pair(vertex_cover.size(), vertex_cover);
}

/*
APRROX VC 2
*/
pair<int, vector<int>> approxVC2(vector<pair<int, int>>& originalE) {
    vector<pair<int, int>> E = originalE; // make a copy
    unordered_set<int> vertex_cover;

    while (!E.empty()) {
        // pick an edge
        int u = E.back().first;
        int v = E.back().second;
        // add both u and v to the vertex cover
        vertex_cover.insert(u);
        vertex_cover.insert(v);
        // remove edges attached to u and v
        E.erase(remove_if(E.begin(), E.end(), [&u, &v](const pair<int, int>& edge) {
            return edge.first == u || edge.first == v || edge.second == u || edge.second == v;
        }), E.end());
    }
    // convert the unordered_set to a vector for the result
    vector<int> vertex_cover_vector(vertex_cover.begin(), vertex_cover.end());
    // sort
    sort(vertex_cover_vector.begin(), vertex_cover_vector.end());

    return {vertex_cover_vector.size(), vertex_cover_vector};
}
