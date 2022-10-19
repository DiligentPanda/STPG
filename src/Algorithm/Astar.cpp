#include "Astar.h"

int heuristic(Simulator simulator) {
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

      return (g1 + h1) > (g2 + h2);
    }
};

bool detectCycleAll(ADG adg) {
  for (int agent = 0; agent < get_agentCnt(adg); agent++) {
    if (detectCycle(adg, agent, 0)) {

      std::cout <<"cycle found for agent " <<agent<<"\n";
      if (detectCycle(adg, 0, 35)) {
        std::cout <<"able to find with 0, 35\n";
      } else {
        std::cout <<"not able to find with 0, 35\n";
      }
      return true;
    }
  }
  return false;
}

ADG exploreNode(priority_queue<Node, vector<Node>, Compare> pq) {
  int newNode = 0;
  int dfsPrune = 0;
  int hPrune = 0;
  while (pq.size() > 0) {
    newNode ++;
    Node node = pq.top();
    pq.pop();
    Simulator simulator = get<0>(node);
    int g = get<1>(node);
    if (newNode >= -10000) {
      std::cout << "newnodecnt: "<<newNode << "\n";
      std::cout << "pq length: " << pq.size() << "\n";
      ADG adg = simulator.adg;
      vector<int> sts = simulator.states;
      for (int agent = 0; agent < get_agentCnt(adg); agent++) {
        std::cout << "agent "<<agent << ": ";
        for (int i = 0; i <= sts[agent]; i++) {
          Location l = get_state_target(adg, agent, i);
          std::cout << "(" << get<0>(l) << ", " << get<1>(l) << ") => ";
        }
        std::cout << "   --- <<" << sts[agent] << " / " << get_stateCnt(adg, agent) << ">> \n\n";
      }
    }

    if (newNode >= -10000) {
      ADG adg = simulator.adg;
      std::cout << "node fix: \n";
      for (int agent = 0; agent < get_agentCnt(adg); agent++) {
        for (int i = 0; i < get_stateCnt(adg, agent); i++) {
          for (pair<int, int> outNeigb: get_nonSwitchable_outNeibPair(adg, agent, i)) {
            if (get<0>(outNeigb) != agent) {
              std::cout << "(" << agent << ", "<< i << ") -> ("<< get<0>(outNeigb) << ", " << get<1>(outNeigb) <<")\n";
            }
          }
        }
      }
      std::cout << "g = " << get<1>(node) << ", h = " << get<2>(node) << "\n";
    }

    int agent1, state1, agent2, state2;
    if (newNode >= -10000) { std::cout << "going to detect switch\n"; }
    tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
    while (agent1 < 0) {
      if (newNode >= -10000) { std::cout << "going to step\n"; }
      int g_step = simulator.step(true);
      if (newNode >= -10000) { std::cout << "stepped\n"; }
      if (g_step == 0) {
        std::cout << "returning, hprune = " << hPrune << ", dfsPrune = " << dfsPrune << "\n";
        return simulator.adg; // All agents reach their goals
      }
      assert(g_step > 0);
      if (newNode >= -10000) {
        std::cout << "stepped, printing stepped simulator:\n";
        ADG adg = simulator.adg;
        vector<int> sts = simulator.states;
        for (int agent = 0; agent < get_agentCnt(adg); agent++) {
          std::cout << "agent "<<agent << ": ";
          for (int i = 0; i <= sts[agent]; i++) {
            Location l = get_state_target(adg, agent, i);
            std::cout << "(" << get<0>(l) << ", " << get<1>(l) << ") => ";
          }
          std::cout << "   --- <<" << sts[agent] << " / " << get_stateCnt(adg, agent) << ">> \n\n";
        }
      }
      g += g_step;
      tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
    }
    if (newNode > -10000) std::cout << "will copy\n";
    // Detected a switchable edge
    ADG copy = copy_ADG(simulator.adg);
    if (newNode > -10000) std::cout << "copied\n";
    // Forward child
    fix_type2_edge(simulator.adg, agent1, state1, agent2, state2);
    if (newNode > -10000) std::cout << "forward fixed\n";
    if (detectCycleAll(simulator.adg)) // Prune node
    {
      std::cout <<"cycle supposed to run for (" << agent1 <<", "<< state1<<") to (" << agent2 << ", " << state2 <<")\n";
      dfsPrune ++;
      std::cout << "forward detected cycle\n";
      free_underlying_graph(simulator.adg);
      if (newNode > -10000) std::cout << "forward freed\n";
    } 
    else
    {
      Simulator simulator_h(simulator.adg, simulator.states);
      int h = heuristic(simulator_h);
      if (h < 0) // Prune node
      {
        hPrune ++;
        free_underlying_graph(simulator.adg);
      } else {
        if (newNode >= -10000) {
          std::cout << "added forward child, fix (" << agent1 << ", "<< state1 << ") -> ("<< agent2 << ", " << state2 <<")\n";
          std::cout << "h = " << h << "\n";
        }
        pq.push(make_tuple(simulator, g, h));
      }
    }

    // Backward child
    fix_type2_edge_reversed(copy, agent1, state1, agent2, state2);
    if (detectCycleAll(copy)) // Prune node
    {
      std::cout <<"cycle supposed to run for (" << agent2 <<", "<< state2<<") to (" << agent1 << ", " << state1 <<")\n";
      dfsPrune++;
      std::cout << "backward detected cycle\n";
      free_underlying_graph(copy);
      exit(0);
    }
    else // Add to the priority queue
    {
      Simulator simulator_h(copy, simulator.states);
      int h = heuristic(simulator_h);
      if (h < 0) // Prune node
      {
        hPrune++;
        free_underlying_graph(copy);
      } else {
        if (newNode >= -10000) {
          std::cout << "added backward child, fix (" << agent2 << ", "<< state2 << ") -> ("<< agent1 << ", " << state1 <<")\n";
          std::cout << "h = " << h << "\n";
        }
        Simulator simulator_back(copy, simulator.states);
        pq.push(make_tuple(simulator_back, g, h)); 
      }
    }
  }
  throw invalid_argument("no solution found");
}

ADG Astar(ADG root) {
  for (int agent = 0; agent < get_agentCnt(root); agent++) {
    for (pair<int, int> outNeigb: get_switchable_outNeibPair(root, agent, 0)) {
      // Fix starting edge
      fix_type2_edge(root, agent, 0, get<0>(outNeigb), get<1>(outNeigb));
    }

    for (pair<int, int> inNeigb: get_switchable_inNeibPair(root, agent, get_stateCnt(root, agent)-1)) {
      // Fix ending edge
      fix_type2_edge(root, get<0>(inNeigb), get<1>(inNeigb), agent, get_stateCnt(root, agent)-1);
    }
  }
  
  Simulator simulator(root);
  priority_queue<Node, vector<Node>, Compare> pq;
  Simulator simulator_h(simulator.adg, simulator.states);
  int h = heuristic(simulator_h);
  pq.push(make_tuple(simulator, 0, h));
  return exploreNode(pq);
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  ADG adg = construct_ADG(fileName);
  ADG res = Astar(adg);
  std::cout<<"finished, result graph: \n";

  Simulator simulator(res);
  int timeSum = heuristic(simulator);
  std::cout << "solution time spend = " << timeSum << "\n";

  ADG original_adg = construct_ADG(fileName);
  set_switchable_nonSwitchable(get<0>(original_adg));
  Simulator original_simulator(original_adg);
  int original_timeSum = heuristic(original_simulator);
  std::cout << "optimal time spend = " << original_timeSum << "\n";
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