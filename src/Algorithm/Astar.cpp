#include "Astar.h"

int heuristic(Simulator simulator) {
  std::cout << "heuristic\n";
  int stepSpend = 0;
  int totalSpend = 0;

  stepSpend = simulator.step(false);
  while (stepSpend != 0) {
    if (stepSpend < 0) return -1; // stuck
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

      return (g1 + h1) <= (g2 + h2);
    }
};

Simulator exploreNode(priority_queue<Node, vector<Node>, Compare> pq,
                      Simulator simulator, int g, int newNode, microseconds m1, microseconds m2, microseconds m3, microseconds m4, microseconds m5) {
                        newNode += 1;
                        if (newNode %1 == 0) {
                          // std::cout << "newnodecnt: "<<newNode << "\n";
                          // std::cout << "pq size: "<<pq.size() << "\n";
                          // std::cout << m1.count() << " m1 \n";
                          // std::cout << m2.count() << " m2 \n";
                          // std::cout << m3.count() << " m3 \n";
                          // std::cout << m4.count() << " m4 \n";
                          // std::cout << m5.count() << " m5 \n";
                        } 
  int agent1, state1, agent2, state2;
  auto start = high_resolution_clock::now();
  tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);
  m1 += duration;
  while (agent1 < 0) {
    start = high_resolution_clock::now();
    int g_step = simulator.step(true);
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    m2 += duration;
    if (g_step == 0) {
      std::cout << "returning\n";
      return simulator; // All agents reach their goals
    }
  //   else if (g_step < 0) {
  //       Node node = pq.top();
  // pq.pop();
  // Simulator new_simulator = get<0>(node);
  // int new_g = get<1>(node);
  // return exploreNode(pq, new_simulator, new_g, newNode);
  //   }
    g += g_step;
    start = high_resolution_clock::now();
    tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    m1 += duration;
  }
  
  // Detected a switchable edge
  start = high_resolution_clock::now();
  ADG copy = copy_ADG(simulator.adg);
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  m4 += duration;
  // Forward child
  fix_type2_edge(simulator.adg, agent1, state1, agent2, state2);
  start = high_resolution_clock::now();
  bool hascycle = detectCycle(simulator.adg, agent1, state1);
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  m5 += duration;
  if (hascycle) // Prune node
  {
    free_underlying_graph(simulator.adg);
  } 
  else
  {
    Simulator simulator_h(simulator.adg, simulator.states);
    start = high_resolution_clock::now();
    int h = heuristic(simulator_h);
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    m3 += duration;
    if (h < 0) // Prune node
    {
      free_underlying_graph(simulator.adg);
    } else {
      pq.push(make_tuple(simulator, g, h));
    }
  }

  // Backward child
  fix_type2_edge_reversed(copy, agent1, state1, agent2, state2);
  start = high_resolution_clock::now();
  hascycle = detectCycle(copy, agent2, state2);
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  m5 += duration;
  if (hascycle) // Prune node
  {
    free_underlying_graph(copy);
  }
  else // Add to the priority queue
  {
    Simulator simulator_h(copy, simulator.states);
    start = high_resolution_clock::now();
    int h = heuristic(simulator_h);
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    m3 += duration;
    if (h < 0) // Prune node
    {
      free_underlying_graph(copy);
    } else {
      Simulator simulator_back(copy, simulator.states);
      pq.push(make_tuple(simulator_back, g, h)); 
    }
  }

  // Recursive call
  Node node = pq.top();
  pq.pop();
  Simulator new_simulator = get<0>(node);
  int new_g = get<1>(node);
  return exploreNode(pq, new_simulator, new_g, newNode, m1, m2, m3, m4, m5);
}

ADG Astar(ADG root) {
  Simulator simulator(root);
  priority_queue<Node, vector<Node>, Compare> pq;
  int g = 0;
  microseconds m1(0); 
  microseconds m2(0); 
  microseconds m3(0); 
  microseconds m4(0); 
  microseconds m5(0); 
  simulator = exploreNode(pq, simulator, g, 0, m1, m2,m3 ,m4, m5);
  return simulator.adg;
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  ADG adg = construct_ADG(fileName);
  print_graph_s2(get<0>(adg));
  ADG res = Astar(adg);
  std::cout<<"finished";
  print_graph_n2(get<0>(res));
  // for (pair<int, int> as: get_switchable_inNeibPair(adg, 13, 1)) {
  //   std::cout << get<0>(as) << ", " << get<1>(as) << ";  ";
  // }
  // Simulator simulator(adg);

  // int agent1, state1, agent2, state2;
  // int step = 0;
  // int g = 0;
  // tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
  // while (agent1 < 0) {
  //   int g_step = simulator.step(true);
  //   if (g_step == 0) {
  //     std::cout << "reach goal\n";
  //     return 0;
  //   }
  //   step ++;
  //   g += g_step;

  //   tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
  // }
  // if (agent1 >= 0) // Detected a switchable edge
  // {
  //   std::cout << "detected switchable: " << agent1 << ", " << state1 << ";  " << agent2 << ", " << state2 << "\n";
  //   std::cout << "step = " << step << ", g = " << g << "\n";
  // }

  return 0;
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