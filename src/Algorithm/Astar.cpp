#include "Astar.h"

int calcTime(Simulator simulator) {
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

int heuristic(ADG adg, vector<int> *ts) {
  Graph graph = get<0>(adg);
  int agentCnt = get_agentCnt(adg);

  vector<int> values((get<2>(adg)).back(), 0);
  for (int i: (*ts)) {
    int prevVal = values[i];
    set<int> outNeib = get_nonSwitchable_outNeib(graph, i);
    for (auto it = outNeib.begin(); it != outNeib.end(); it++) {
      int j = *it;
      int weight = 1;
      if (values[j] < prevVal + weight) values[j] = prevVal + weight;
    }
  }

  int sum = 0;
  for (int agent = 0; agent < agentCnt; agent ++) {
    int goalVert = compute_vertex(get<2>(adg), agent, get_stateCnt(adg, agent) - 1);
    sum += values[goalVert];
  }
  return sum;
}

class Compare {
  public:
    bool operator() (Node s1, Node s2)
    {
      int val1 = get<1>(s1);
      int val2 = get<1>(s2);

      return val1 > val2;
    }
};

ADG exploreNode(priority_queue<Node, vector<Node>, Compare> pq) {
  while (pq.size() > 0) {
    Node node = pq.top();
    pq.pop();
    
    ADG adg = get<0>(node);
    int agentCnt = get_agentCnt(adg);
    Graph graph = get<0>(adg);

    vector<int> currents;
    for (int agent = 0; agent < agentCnt; agent ++) {
      int current = compute_vertex(get<2>(adg), agent, 0);
      currents.push_back(current);
    }
    vector<int>* ts_tv;
    vector<int>* ts_vt;
    tie(ts_tv, ts_vt) = topologicalSort(graph, currents);

    int maxDiff = -1;
    int maxI, maxJ;
    for (int i = 0; i < get<3>(graph); i++) {
      int iTime = (*ts_vt)[i];
      set<int>& outNeib = get_switchable_outNeib(graph, i);
      for (auto it = outNeib.begin(); it != outNeib.end(); it++) {
        int j = *it;
        int jTime = (*ts_vt)[j];
        if (jTime > iTime) { // No need to revert
          // Fix the edge
          rem_type2_switchable_edge(graph, i, j);
          set_type2_nonSwitchable_edge(graph, i, j);
        } else {
          int diff = iTime - jTime;
          if (diff > maxDiff) {
            maxDiff = diff;
            maxI = i;
            maxJ = j;
          }
        }
      }
    }

    if (maxDiff < 0) { // All switchable obeys the sort
      return adg;
    } else {
      ADG copy = copy_ADG(adg);

      // Forward
      
      // Fix the edge
      rem_type2_switchable_edge(graph, maxI, maxJ);
      set_type2_nonSwitchable_edge(graph, maxI, maxJ);

      if (check_cycle_nonSwitchable(graph, maxI)) { // Prune node
        free_graph(graph);
      } else {
        vector<int>* newts_tv;
        vector<int>* newts_vt;
        tie(newts_tv, newts_vt) = topologicalSort(graph, currents);
        int val = heuristic(adg, newts_tv);
        pq.push(make_tuple(adg, val));
      }

      // Backward 
      Graph copyGraph = get<0>(copy);

      // Fix the edge
      int backI = maxJ+1;
      int backJ = maxI-1;
      assert((*ts_vt)[backJ] > (*ts_vt)[backI]); // (should obey the original sort)

      rem_type2_switchable_edge(copyGraph, backI, backJ);
      set_type2_nonSwitchable_edge(copyGraph, backI, backJ);

      if (check_cycle_nonSwitchable(copyGraph, backI)) { // Prune node
        free_graph(copyGraph);
      } else {
        int val = heuristic(copy, ts_tv);
        pq.push(make_tuple(copy, val));
      }
    }
  }
  throw invalid_argument("no solution found");
}

ADG Astar(ADG adg) {
  priority_queue<Node, vector<Node>, Compare> pq;
  pq.push(make_tuple(adg, 0));
  return exploreNode(pq);
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  ADG adg = construct_ADG(fileName);

  for (int agent = 0; agent < get_agentCnt(adg); agent++) {
    for (pair<int, int> outNeigb: get_switchable_outNeibPair(adg, agent, 0)) {
      // Fix starting edge
      fix_type2_edge(adg, agent, 0, get<0>(outNeigb), get<1>(outNeigb));
    }

    for (pair<int, int> inNeigb: get_switchable_inNeibPair(adg, agent, get_stateCnt(adg, agent)-1)) {
      // Fix ending edge
      fix_type2_edge(adg, get<0>(inNeigb), get<1>(inNeigb), agent, get_stateCnt(adg, agent)-1);
    }
  }
  
  ADG res = Astar(adg);
  std::cout<<"finished, result graph: \n";

  Simulator simulator_res(res);
  int timeSum = calcTime(simulator_res);
  std::cout << "solution time spend = " << timeSum << "\n";

  ADG original_adg = construct_ADG(fileName);
  set_switchable_nonSwitchable(get<0>(original_adg));
  Simulator original_simulator(original_adg);
  int original_timeSum = calcTime(original_simulator);
  std::cout << "optimal time spend = " << original_timeSum << "\n";
  return 0;
}