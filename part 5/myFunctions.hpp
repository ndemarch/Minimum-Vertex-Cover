#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <vector>
#include <utility>

/*
Function to perform CNF reduction
Parameters:
  n: Number of vertices
  k: Parameter for the reduction
  edges: List of edges in the graph
  timeout_seconds: Timeout for the operation in seconds
Returns:
  A vector representing the result of CNF reduction
*/
std::vector<int> cnfReduction(int n, int k, const std::vector<std::pair<int, int>>& edges, int timeout_seconds);

/*
Function to perform binary search coverage
Parameters:
  n: Number of vertices
  edges: List of edges in the graph
  timeout_seconds: Timeout for the operation in seconds
Returns:
  A pair representing the result of binary search coverage, including the coverage size and vertices
*/
std::pair<int, std::vector<int>> binarySearchCoverage(int n, const std::vector<std::pair<int, int>>& edges, int timeout_seconds);

/*
Function to perform the APPROX-VC-1 algorithm for finding a vertex cover.
Parameters:
  E: List of edges in the graph (updated to reflect the remaining edges after the algorithm)
Returns:
  A pair representing the result of the APPROX-VC-1 algorithm, including the vertex cover and its size.
  The vertex cover is a vector containing the selected vertices, and the size is the number of vertices in the cover.
*/
std::pair<int, std::vector<int>> approxVC1(std::vector<std::pair<int, int>>& originalE);

/*
Function to perform the APPROX-VC-1 algorithm for finding a vertex cover.
This function takes a vector of edges representing a graph and returns an approximate
minimum vertex cover using the APPROXVC-2 algorithm.
Parameters:
  - E: A vector of pairs representing edges in the graph.
Returns:
  A pair containing the approximate vertex cover (as an unordered set of vertices) and
  the size of the vertex cover.
 */
std::pair<int, std::vector<int>> approxVC2(std::vector<std::pair<int, int>>& originalE);

#endif
