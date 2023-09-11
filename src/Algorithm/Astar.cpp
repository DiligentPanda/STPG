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
  int cnt = 0;
  int pruned = 0;
  int added = 0;
  
  microseconds timer(0);

  while (pq.size() > 0) {
    cnt += 1;
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

    vector<int> &ts_vt = get<2>(node);

    int maxDiff = -1;
    int maxI = -1;
    int maxJ = -1;
    for (int i = 0; i < get<3>(graph); i++) {
      int iTime = ts_vt[i];
      set<int> outNeib = get_switchable_outNeib(graph, i);
      for (auto it : outNeib) {
        int j = it;
        int jTime = ts_vt[j];
        int diff = iTime - jTime;
        if (diff > maxDiff) {
          maxDiff = diff;
          maxI = i;
          maxJ = j;
          
        }
      }
    }

    if (maxDiff < 0) { 
      std::cout << "------------------------SORT TAKE TIME:::: " << timer.count() << "\n\n";
      std::cout << "cnt=" << cnt << "\n" << std::flush;
      std::cout << "pruned=" << pruned << "\n" << std::flush;
      std::cout << "added=" << added << "\n" << std::flush;
      return adg;
    } else {
      ADG copy = copy_ADG(adg);

      // Forward
      
      // Fix the edge
      rem_type2_switchable_edge(graph, maxI, maxJ);
      set_type2_nonSwitchable_edge(graph, maxI, maxJ);

      if (check_cycle_nonSwitchable(graph, maxI)) { // Prune node
        free_graph(graph);
        pruned += 1;
      } else {
        vector<int>* newts_tv_init = nullptr;
        vector<int>* newts_vt_init = nullptr;
        sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

        vector<int>* newts_tv;
        vector<int>* newts_vt;
        
        auto start = high_resolution_clock::now();
        tie(newts_tv, newts_vt) = topologicalSort(graph, newInitResult, &currents);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        timer += duration;

        int val = heuristic(adg, newts_tv);
        pq.push(make_tuple(adg, val, *newts_vt));
        added += 1;
      }

      // Backward 
      Graph copyGraph = get<0>(copy);

      // Fix the edge
      int backI = maxJ+1;
      int backJ = maxI-1;

      rem_type2_switchable_edge(copyGraph, maxI, maxJ);
      set_type2_nonSwitchable_edge(copyGraph, backI, backJ);

      if (check_cycle_nonSwitchable(copyGraph, backI)) { // Prune node
        free_graph(copyGraph);
      } else {
        vector<int>* newts_tv_init = nullptr;
        vector<int>* newts_vt_init = nullptr;
        sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

        vector<int>* newts_tv;
        vector<int>* newts_vt;

        auto start = high_resolution_clock::now();
        tie(newts_tv, newts_vt) = topologicalSort(copyGraph, newInitResult, &currents);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        timer += duration;

        int val = heuristic(copy, newts_tv);
        pq.push(make_tuple(copy, val, *newts_vt));
        added += 1;
      }
    }
  }
  std::cout << "cnt=" << cnt << "\n" << std::flush;
  std::cout << "pruned=" << pruned << "\n" << std::flush;
  std::cout << "added=" << added << "\n" << std::flush;
  throw invalid_argument("no solution found");
}

ADG Astar(ADG adg) {
  priority_queue<Node, vector<Node>, Compare> pq;

  int agentCnt = get_agentCnt(adg);
  Graph graph = get<0>(adg);

  vector<int> currents;
  for (int agent = 0; agent < agentCnt; agent ++) {
    int current = compute_vertex(get<2>(adg), agent, 0);
    currents.push_back(current);
  }
  
  vector<int>* ts_tv_init = nullptr;
  vector<int>* ts_vt_init = nullptr;
  sortResult initResult = make_pair(ts_tv_init, ts_vt_init);

  sortResult result = topologicalSort(graph, initResult, &currents);
  vector<int>* ts_vt = result.second;

  pq.push(make_tuple(adg, 0, *ts_vt));
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
  
  microseconds timer(0);
  auto start = high_resolution_clock::now();
  ADG res = Astar(adg);
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);
  timer += duration;
  std::cout << "------------------------REPLANNING TAKE TIME:::: " << timer.count() << "\n\n";
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