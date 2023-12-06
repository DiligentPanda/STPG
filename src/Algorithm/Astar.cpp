#include "Astar.h"

Astar::Astar() {
}

Astar::Astar(int input_timeout) {
  timeout = input_timeout;
}

Astar::Astar(int input_timeout, bool input_term_shortcut) {
  timeout = input_timeout;
  term_shortcut = input_term_shortcut;
}


int Astar::calcTime(Simulator simulator) {
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


int Astar::heuristic_graph(ADG &adg, vector<int> *ts, vector<int> *values) {
  Graph &graph = get<0>(adg);

  for (int i: (*ts)) {
    int prevVal = values->at(i);
    set<int> outNeib = get_nonSwitchable_outNeib(graph, i);
    for (auto it = outNeib.begin(); it != outNeib.end(); it++) {
      int j = *it;
      int weight = 1;
      if (values->at(j) < prevVal + weight) values->at(j) = prevVal + weight;
    }
  }
  int sum = 0;
  for (int agent = 0; agent < agentCnt; agent ++) {
    int goalVert = compute_vertex(get<2>(adg), agent, get_stateCnt(adg, agent) - 1);
    sum += values->at(goalVert);
  }
  return sum;
}

tuple<int, int, int> Astar::branch(Graph &graph, vector<int> *values) {
  int maxDiff = -1;
  int maxI = -1;
  int maxJ = -1;
  for (int i = 0; i < get<3>(graph); i++) {
    int iTime = values->at(i);
    set<int> &outNeib = get_switchable_outNeib(graph, i);
    for (auto it : outNeib) {
      int j = it;
      int jTime = values->at(j);
      int diff = iTime - jTime;
      if (diff > maxDiff) {
        maxDiff = diff;
        maxI = i;
        maxJ = j;
      }
      if (maxDiff > 0) {
        break;
      } 
    }
    if (maxDiff > 0) {
      break;
    } 
  }
  return make_tuple(maxDiff, maxI, maxJ);
}

void Astar::print_stats(ofstream &outFile) {
    outFile << explored_node_cnt << "," << 
    pruned_node_cnt << "," << 
    added_node_cnt << "," << 
    heuristicT.count() << "," << 
    branchT.count() << "," <<
    sortT.count() << "," << 
    pqT.count() << "," << 
    copy_free_graphsT.count() << "," << 
    dfsT.count() << endl;
}

void Astar::print_stats() {
    std::cout << "explored_node_cnt =" << explored_node_cnt << "\n";
    std::cout << "pruned_node_cnt =" << pruned_node_cnt << "\n";
    std::cout << "added_node_cnt =" << added_node_cnt << "\n\n";

    std::cout << "-------------------Time breakdown: \n";
    std::cout << "heuristic time: " << heuristicT.count() << "\n";
    std::cout << "branch time: " << branchT.count() << "\n";
    std::cout << "sort time: " << sortT.count() << "\n";
    std::cout << "pq time: " << pqT.count() << "\n";
    std::cout << "copy_free_graphs time: " << branchT.count() << "\n";
    std::cout << "dfs time: " << dfsT.count() << "\n";
}

ADG Astar::exploreNode() {
  auto start = high_resolution_clock::now();
  while (pq.size() > 0) {
    explored_node_cnt += 1;
    if (explored_node_cnt % 5000 == 0) {
      std::cout << "cnt=" << explored_node_cnt << "\n" << std::flush;
    }

    auto start_pq_pop = high_resolution_clock::now();
    Node* node = pq.top();
    pq.pop();
    auto end_pq_pop = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
    
    ADG &adg = get<0>(*node);
    vector<int> *values = get<2>(*node);
    Graph &graph = get<0>(adg);

    int maxDiff, maxI, maxJ;
    auto start_branch = high_resolution_clock::now();
    tie(maxDiff, maxI, maxJ) = branch(graph, values);
    auto end_branch = high_resolution_clock::now();
    branchT += duration_cast<microseconds>(end_branch - start_branch);

    bool terminate = true;

    if (term_shortcut) {
      terminate = (maxDiff < 0);
    } else {
      for (int v = 0; v < get<3>(graph); v ++) {
        set<int>& outNeib = get_switchable_outNeib(graph, v);
        if (outNeib.size() != 0) {
          terminate = false;
          maxI = v;
          maxJ = *(outNeib.begin());
          break;
        }
      }
    }

    if (terminate || ((duration_cast<seconds>(end_branch - start)).count() >= timeout)) {
      while (pq.size() > 0) {
        start_pq_pop = high_resolution_clock::now();
        Node* delete_node = pq.top();
        end_pq_pop = high_resolution_clock::now();
        free_underlying_graph(get<0>(*delete_node));
        auto end_graph_free = high_resolution_clock::now();

        pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - end_pq_pop);

        vector<int> *delete_vec = get<2>(*delete_node);
        delete delete_vec;
        delete delete_node;

        start_pq_pop = high_resolution_clock::now();
        pq.pop();
        end_pq_pop = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
      }
      auto start_graph_free = high_resolution_clock::now();
      ADG res = copy_ADG(adg);
      free_underlying_graph(adg);
      auto end_graph_free = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);

      delete values;
      delete node;

      set_switchable_nonSwitchable(get<0>(res));
      return res;
    } else {
      auto start_graph_copy = high_resolution_clock::now();
      ADG copy = copy_ADG(adg);
      auto end_graph_copy = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_copy - start_graph_copy);

      // Forward
      // Fix the edge
      rem_type2_switchable_edge(graph, maxI, maxJ);
      set_type2_nonSwitchable_edge(graph, maxI, maxJ);

      auto start_dfs = high_resolution_clock::now();
      bool pruned_forward = check_cycle_dfs(graph, maxI);
      auto end_dfs = high_resolution_clock::now();
      dfsT += duration_cast<microseconds>(end_dfs - start_dfs);

      if (pruned_forward) { // Prune node
        auto start_graph_free = high_resolution_clock::now();
        free_graph(graph);
        auto end_graph_free = high_resolution_clock::now();
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
        pruned_node_cnt += 1;
      } else {
        vector<int>* newts_tv_init = nullptr;
        vector<int>* newts_vt_init = nullptr;
        sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

        vector<int>* newts_tv;
        vector<int>* newts_vt;
        vector<int>* node_values = new vector<int>(get<3>(graph), 0);

        auto start_sort = high_resolution_clock::now();
        tie(newts_tv, newts_vt) = topologicalSort(graph, newInitResult, &currents, -1, -1);
        auto end_sort = high_resolution_clock::now();
        int val = heuristic_graph(adg, newts_tv, node_values);
        auto end_heuristic = high_resolution_clock::now();

        sortT += duration_cast<microseconds>(end_sort - start_sort);
        heuristicT += duration_cast<microseconds>(end_heuristic - end_sort);

        delete newts_tv;
        delete newts_vt;

        Node *forward_node = new Node;
        *forward_node = make_tuple(adg, val, node_values);

        auto start_pq_push = high_resolution_clock::now();
        pq.push(forward_node);
        auto end_pq_push = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

        added_node_cnt += 1;
      }

      // Backward 
      Graph &copyGraph = get<0>(copy);

      // Fix the edge
      int backI = maxJ+1;
      int backJ = maxI-1;

      rem_type2_switchable_edge(copyGraph, maxI, maxJ);
      set_type2_nonSwitchable_edge(copyGraph, backI, backJ);

      start_dfs = high_resolution_clock::now();
      bool pruned_backward = check_cycle_dfs(copyGraph, backI);
      end_dfs = high_resolution_clock::now();
      dfsT += duration_cast<microseconds>(end_dfs - start_dfs);

      if (pruned_backward) { // Prune node
        auto start_graph_free = high_resolution_clock::now();
        free_graph(copyGraph);
        auto end_graph_free = high_resolution_clock::now();
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
        pruned_node_cnt += 1;
      } else {
        vector<int>* newts_tv_init = nullptr;
        vector<int>* newts_vt_init = nullptr;
        sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

        vector<int>* newts_tv;
        vector<int>* newts_vt;
        vector<int>* node_values = new vector<int>(get<3>(copyGraph), 0);

        auto start_sort = high_resolution_clock::now();
        tie(newts_tv, newts_vt) = topologicalSort(copyGraph, newInitResult, &currents, -1, -1);
        auto end_sort = high_resolution_clock::now();
        int val = heuristic_graph(copy, newts_tv, node_values);
        auto end_heuristic = high_resolution_clock::now();

        sortT += duration_cast<microseconds>(end_sort - start_sort);
        heuristicT += duration_cast<microseconds>(end_heuristic - end_sort);

        delete newts_tv;
        delete newts_vt;

        Node *backward_node = new Node;
        *backward_node = make_tuple(copy, val, node_values);

        auto start_pq_push = high_resolution_clock::now();
        pq.push(backward_node);
        auto end_pq_push = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

        added_node_cnt += 1;
      }
    }
    delete values;
    delete node;
  }
  throw invalid_argument("no solution found");
}

ADG Astar::startExplore(ADG &adg) {
  agentCnt = get_agentCnt(adg);
  Graph &graph = get<0>(adg);

  for (int agent = 0; agent < agentCnt; agent ++) {
    int current = compute_vertex(get<2>(adg), agent, 0);
    currents.push_back(current);
  }
  
  vector<int>* ts_tv_init = nullptr;
  vector<int>* ts_vt_init = nullptr;
  sortResult initResult = make_pair(ts_tv_init, ts_vt_init);

  auto start_sort = high_resolution_clock::now();
  sortResult result = topologicalSort(graph, initResult, &currents, -1, -1);
  auto end_sort = high_resolution_clock::now();
  sortT += duration_cast<microseconds>(end_sort - start_sort);

  delete result.second;
  vector<int>* ts_tv = result.first;
  vector<int>* node_values = new vector<int>(get<3>(graph), 0);

  auto start_heuristic = high_resolution_clock::now();
  int val = heuristic_graph(adg, ts_tv, node_values);
  auto end_heuristic = high_resolution_clock::now();
  heuristicT += duration_cast<microseconds>(end_heuristic - start_heuristic);
  delete ts_tv;

  Node *root = new Node;
  *root = make_tuple(adg, val, node_values);

  auto start_pq_push = high_resolution_clock::now();
  pq.push(root);
  auto end_pq_push = high_resolution_clock::now();
  pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

  return exploreNode();
}

// int main(int argc, char** argv) {
//   char* fileName = argv[1];
//   ADG adg = construct_ADG(fileName);
//   microseconds timer(0);
//   auto start = high_resolution_clock::now();
//   Astar search;

//   ADG res = search.startExplore(adg);
//   auto stop = high_resolution_clock::now();
//   auto duration = duration_cast<microseconds>(stop - start);
//   timer += duration;
//   std::cout << "------------------------REPLANNING TAKE TIME:::: " << timer.count() << "\n\n";
//   std::cout<<"finished, result graph: \n";

//   Simulator simulator_res(res);

//   int timeSum = simulator_res.print_soln("out.txt");
//   std::cout << "solution time spend = " << timeSum << "\n";

//   int agentCnt = get_agentCnt(res);
//   vector<int> currents;
//   for (int agent = 0; agent < agentCnt; agent ++) {
//     int current = compute_vertex(get<2>(res), agent, 0);
//     currents.push_back(current);
//   }
//   vector<int>* newts_tv_init = nullptr;
//   vector<int>* newts_vt_init = nullptr;
//   sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

//   vector<int>* newts_tv;
//   vector<int>* newts_vt;
//   tie(newts_tv, newts_vt) = topologicalSort(get<0>(res), newInitResult, &currents, -1, -1);
//   vector<int> newValues(get<3>(get<0>(res)), 0);
//   int hVal = search.heuristic_graph(res, newts_tv, &newValues);
//   delete newts_tv;
//   delete newts_vt;
//   std::cout << "solution final h val = " << hVal << "\n";


//   ADG original_adg = construct_ADG(fileName)
//   set_switchable_nonSwitchable(get<0>(original_adg));
//   Simulator original_simulator(original_adg);
//   int original_timeSum = original_simulator.print_soln("ori.txt");
//   std::cout << "original time spend = " << original_timeSum << "\n";

//   newts_tv_init = nullptr;
//   newts_vt_init = nullptr;
//   newInitResult = make_pair(newts_tv_init, newts_vt_init);

//   vector<int>* newts_tv2;
//   vector<int>* newts_vt2;
//   tie(newts_tv2, newts_vt2) = topologicalSort(get<0>(original_adg), newInitResult, &currents, -1, -1);
//   vector<int> newValues2(get<3>(get<0>(original_adg)), 0);
//   int hVal2 = search.heuristic_graph(original_adg, newts_tv2, &newValues2);
//   delete newts_tv2;
//   delete newts_vt2;
//   std::cout << "original final h val = " << hVal2 << "\n";


//   free_underlying_graph(original_adg);
//   free_shared_graph(original_adg);
//   free_underlying_graph(res);
//   free_shared_graph(res);
//   return 0;
// }