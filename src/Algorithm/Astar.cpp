#include "Astar.h"

// Dead code below for longest path heuristic algorithms. Might restore later
// void topologicalSort(ADG adg, int v, bool* visited, vector<int> result)
// {
//   if (v >= 0) visited[v] = true;
//   vector<int> neighbors;
//   if (v >= 0) {
//     Graph graph = get<0>(adg);
//     neighbors = get_outNeighbors(graph, v);
//   } else {
//     for (int i = 0; i < get_agentCnt(adg); i++) {
//       neighbors.push_back(compute_vertex_ADG(adg, i, 0));
//     }
//   }

//   for (int neighbor: neighbors) {
//     if (!visited[neighbor]) {
//       topologicalSort(adg, neighbor, visited, result);
//     }
//   }
//   result.push_back(v);
// }

// int heuristic(ADG adg) {
//   int n = get_totalStateCnt(adg);
//   vector<int> result;
//   bool* visited = new bool[n]();
//   int v = -1;
//   topologicalSort(adg, v, visited, result);
// }

int heuristic(ADG adg) {
  Simulator simulator(adg);
  int stepSpend = 0;
  int totalSpend = 0;

  int stepSpend = simulator.step();
  while (stepSpend != 0) {
    totalSpend += stepSpend;
    stepSpend = simulator.step();
  }
  return totalSpend;
}

ADG Astar(ADG root) {

}