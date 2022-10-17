#include "Astar.h"

int heuristic(Simulator simulator) {
  int stepSpend = 0;
  int totalSpend = 0;

  int stepSpend = simulator.step(false);
  while (stepSpend != 0) {
    totalSpend += stepSpend;
    stepSpend = simulator.step(false);
  }
  return totalSpend;
}

class Compare {
  public:
    bool operator() (Node s1, Node s2)
    {
      int g1 = get<1>(s1);
      int h1 = get<2>(s1);
      int g2 = get<1>(s2);
      int h2 = get<2>(s2);

      return (g1 + h1) < (g2 + h2);
    }
};


ADG Astar(ADG root) {
  Simulator simulator(root);
  priority_queue<Node, vector<Node>, Compare> pq;
  int g = 0;

  int agent1, state1, agent2, state2;
  tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
  if (agent1 >= 0) // Detected a switchable edge
  {
    ADG copy = copy_ADG(simulator.adg);
    // Forward child
    fix_type2_edge(simulator.adg, agent1, state1, agent2, state2);
    if (detectCycle(simulator.adg, agent1, state1)) // Prune node
    {
      free_underlying_graph(simulator.adg);
    } 
    else // Add to the priority queue
    {
      Simulator simulator_h(simulator.adg, simulator.states);
      int h = heuristic(simulator_h);
      pq.push(make_tuple(simulator, g, h));
    }

    // Backward child
    fix_type2_edge_reversed(copy, agent1, state1, agent2, state2);
    if (detectCycle(copy, agent2, state2)) // Prune node
    {
      free_underlying_graph(copy);
    }
     else // Add to the priority queue
    {
      Simulator simulator_h(copy, simulator.states);
      int h = heuristic(simulator_h);
      Simulator simulator(copy, simulator.states);
      pq.push(make_tuple(simulator, g, h)); 
    }
  }
}

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