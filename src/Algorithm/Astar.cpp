#include "Algorithm/Astar.h"
#include <algorithm>
#include <limits.h>
#include "group/group.h"

Astar::Astar() {
}

Astar::Astar(int input_timeout) {
  timeout = input_timeout;
}

Astar::Astar(
    int input_timeout, 
    bool input_fast_version, 
    const string & _branch_order, 
    bool use_grouping, 
    const string & _heuristic, 
    bool early_termination, 
    float _weight_h,
    uint random_seed
  ): rng(random_seed), weight_h(_weight_h) {
  timeout = input_timeout;
  fast_version = input_fast_version;
  if (_branch_order=="default") {
    branch_order=BranchOrder::DEFAULT;
  } else if (_branch_order=="conflict") {
    branch_order=BranchOrder::CONFLICT;
  } else if (_branch_order=="largest_diff") {
    branch_order=BranchOrder::LARGEST_DIFF;
  } else if (_branch_order=="random") {
    branch_order=BranchOrder::RANDOM;
  } else if (_branch_order=="earliest") {
    branch_order=BranchOrder::EARLIEST;
  } else {
    std::cout<<"unknown branch order: "<<_branch_order<<std::endl;
    exit(177);
  }

  HeuristicType heuristic;
  if (_heuristic=="zero") {
    heuristic=HeuristicType::ZERO;
  } else if (_heuristic=="cg_greedy") {
    heuristic=HeuristicType::CG_GREEDY;
  } else if (_heuristic=="wcg_greedy") {
    heuristic=HeuristicType::WCG_GREEDY;
  } else {
    std::cout<<"unknown heuristic: "<<_heuristic<<std::endl;
    exit(18);
  }

  this->use_grouping=use_grouping;
  this->early_termination=early_termination;
  this->heuristic_manager=std::make_shared<HeuristicManager>(heuristic);
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

// compute the longest path length from any current vertices to an agent's goal vertex.
// the heuristic is the sum of the longest path length.
float Astar::heuristic_graph(ADG &adg, shared_ptr<vector<int> > ts, shared_ptr<vector<int> > values) {
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
  float sum = 0;
  for (int agent = 0; agent < agentCnt; agent ++) {
    int goalVert = compute_vertex((*get<2>(adg)), agent, get_stateCnt(adg, agent) - 1);
    sum += values->at(goalVert);
  }

  // TODO(rivers): set time limit;
  // add an extra admissible heuristic
  auto start = high_resolution_clock::now();
  int h= heuristic_manager->computeInformedHeuristics(adg, *ts, *values, 0);
  auto end = high_resolution_clock::now();
  extraHeuristicT += duration_cast<microseconds>(end - start);
  if (sum+h>=init_cost){
    return -1;
  }

  sum+=h*weight_h;

  // int partial_cost = compute_partial_cost(adg);
  // if (partial_cost != sum) {
  //   std::cout << "mismatched! PC = " << partial_cost << ", SUM = " << sum << "\n";
  // }
  return sum;
}

tuple<int, int, int> Astar::branch(Graph &graph, shared_ptr<vector<int> > values) {
  int maxDiff = -1;
  int maxI = -1;
  int maxJ = -1;

  if (branch_order==BranchOrder::DEFAULT) {
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
        // rivers: it is a bug that we only check maxDiff>0, it should be maxDiff>=0
        // rivers: well, it is not a bug, it is just a design choice if termermination checks maxDiff<0.
        if (maxDiff > 0) {
          break;
        } 
      }
      if (maxDiff > 0) {
        break;
      } 
    }
  } else if (branch_order==BranchOrder::LARGEST_DIFF) {
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
      }
    }
  } else if (branch_order==BranchOrder::EARLIEST) {
    int earliest_t=INT_MAX;
    for (int i = 0; i < get<3>(graph); i++) {
      int iTime = values->at(i);
      set<int> &outNeib = get_switchable_outNeib(graph, i);
      for (auto it : outNeib) {
        int j = it;
        int jTime = values->at(j);
        int diff = iTime - jTime;
        if (diff>=0 && jTime<earliest_t) {
          maxDiff = diff;
          maxI = i;
          maxJ = j;
          earliest_t=jTime;
        }
      }
    }
  } else if (branch_order==BranchOrder::RANDOM) {

    std::vector<std::tuple<int,int,int> > tuples;
    for (int i = 0; i < get<3>(graph); i++) {
      int iTime = values->at(i);
      set<int> &outNeib = get_switchable_outNeib(graph, i);
      for (auto it : outNeib) {
        int j = it;
        int jTime = values->at(j);
        int diff = iTime - jTime;
        if (diff>=0) {
          tuples.emplace_back(diff,i,j);
        }
      }
    }

    if (tuples.size()>0) {
      int random_idx=rng()%tuples.size();
      std::tie(maxDiff,maxI,maxJ)=tuples[random_idx];
    }
  } else {
    std::cout<<"unknown branch order"<<std::endl;
    exit(178);
  }

  return make_tuple(maxDiff, maxI, maxJ);
}

tuple<int, int, int> Astar::enhanced_branch(Graph &graph, shared_ptr<vector<int> > values) {
  int maxDiff = -1;
  int maxI = -1;
  int maxJ = -1;
  for (int i = 0; i < get<3>(graph); i++) {
    int iTime = values->at(i);
    set<int> &outNeib = get_switchable_outNeib(graph, i);
    for (auto it : outNeib) {
      int j = it;
      int backI = j+1;
      int backJ = i-1;
      int jTime = values->at(j);
      int backITime = values->at(backI);
      int backJTime = values->at(backJ);
      int diff = iTime - jTime;
      int backDiff = backITime - backJTime;

      // TODO(rivers): should we try to find maxDiff
      if (diff>=0 && backDiff>=0) {
        int _diff = std::min(diff,backDiff);
        if (_diff>maxDiff) {
          maxDiff=_diff;
          maxI=i;
          maxJ=j;
        }
      }
      // rivers: it is at most 1.
      if (maxDiff>=1) {
        break;
      }
    }
    if (maxDiff>=1) {
      break;
    } 
  }
  return make_tuple(maxDiff, maxI, maxJ);
}

bool Astar::terminated(Graph &graph, shared_ptr<vector<int> > values) {
  if (early_termination) {
    for (int i = 0; i < get<3>(graph); i++) {
      int iTime = values->at(i);
      set<int> &outNeib = get_switchable_outNeib(graph, i);
      for (auto it : outNeib) {
        int j = it;
        int backI = j+1;
        int backJ = i-1;
        int jTime = values->at(j);
        int backITime = values->at(backI);
        int backJTime = values->at(backJ);
        int diff = iTime - jTime;
        int backDiff = backITime - backJTime;

        if (diff>=0 && backDiff>=0) {
          return false;
        }
      }
    }
    return true;
  } else {
    for (int i = 0; i < get<3>(graph); i++) {
      int iTime = values->at(i);
      set<int> &outNeib = get_switchable_outNeib(graph, i);
      for (auto it : outNeib) {
        int j = it;
        int jTime = values->at(j);
        int diff = iTime - jTime;
        if (diff >= 0) {
          return false;
        }
      }
    }
    return true;
  }
}

void Astar::print_stats(nlohmann::json & stats) {
  stats["explored_node"]=explored_node_cnt;
  stats["pruned_node"]=pruned_node_cnt;
  stats["added_node"]=added_node_cnt;
  stats["vertex"]=vertex_cnt;
  stats["sw_edge"]=sw_edge_cnt;
  stats["heuristic_time"]=heuristicT.count();
  stats["extra_heuristic_time"]=extraHeuristicT.count();
  stats["branch_time"]=branchT.count();
  stats["sort_time"]=sortT.count();
  stats["priority_queue_time"]=pqT.count();
  stats["copy_free_graphs_time"]=copy_free_graphsT.count();
  stats["termination_time"]=termT.count();
  stats["dfs_time"]=dfsT.count();
  stats["grouping_time"]=groupingT.count();
  if (use_grouping) {
    auto & groups=group_manager->groups;
    stats["group"]=groups.size();
    stats["group_merge_edge"]=group_manager->group_merge_edge_cnt;
    float group_size_max=0;
    float group_size_avg=0;
    float group_size_min=sw_edge_cnt;
    for (int i=0;i<(int)groups.size();++i) {
      if((int)groups[i].size()>group_size_max) {
        group_size_max=groups[i].size();
      }
      if ((int)groups[i].size()<group_size_min) {
        group_size_min=groups[i].size();
      }
      group_size_avg+=groups[i].size();
    }
    group_size_avg/=groups.size();
    stats["group_size_max"]=group_size_max;
    stats["group_size_min"]=group_size_min;
    stats["group_size_avg"]=group_size_avg;
  } else {
    stats["group"]=sw_edge_cnt;
    stats["group_merge_edge"]=0;
    stats["group_size_max"]=1;
    stats["group_size_min"]=1;
    stats["group_size_avg"]=1;
  }
}

void Astar::print_stats(ofstream &outFile) {
    outFile << explored_node_cnt << "," << 
    pruned_node_cnt << "," << 
    added_node_cnt << "," << 
    vertex_cnt << "," << 
    sw_edge_cnt << "," << 
    heuristicT.count() << "," << 
    branchT.count() << "," <<
    sortT.count() << "," << 
    pqT.count() << "," << 
    copy_free_graphsT.count() << "," << 
    termT.count() << "," << 
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

    if (explored_node_cnt % 1000000 == 0) {
      std::cout << explored_node_cnt << "\n";
    }

    auto start_pq_pop = high_resolution_clock::now();
    auto node = pq.top();
    pq.pop();
    auto end_pq_pop = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
    
    ADG &adg = get<0>(*node);
    shared_ptr<vector<int> > values = get<2>(*node);
    Graph &graph = get<0>(adg);

    int maxDiff, maxI, maxJ;
    auto start_branch = high_resolution_clock::now();
    // maxI, maxJ returns a type 2 edge to branch (maxDiff is useless for now)
    if (branch_order==BranchOrder::CONFLICT) {
      tie(maxDiff, maxI, maxJ) = enhanced_branch(graph, values);  
    } else {
      tie(maxDiff, maxI, maxJ) = branch(graph, values);
    }
    auto end_branch = high_resolution_clock::now();
    branchT += duration_cast<microseconds>(end_branch - start_branch);

    bool terminate = true;

    if (true) {
      terminate = terminated(graph, values);
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
    auto end_term = high_resolution_clock::now();
    termT += duration_cast<microseconds>(end_term - end_branch);

    if (terminate || ((duration_cast<seconds>(end_branch - start)).count() >= timeout)) {
      // free all the search node in the priority queue.
      while (pq.size() > 0) {
        pq.pop();
      }

      // return the current ADG by fix switchable edges to non-switchable.
      auto start_graph_free = high_resolution_clock::now();
      ADG res = copy_ADG(adg);
      auto end_graph_free = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);

      if (terminate && early_termination) {
        reverse_nonSwitchable_edges_basedOn_LongestPathValues(get<0>(res), values);
      }

      set_switchable_nonSwitchable(get<0>(res));
      return res;
    } else {
      auto start_graph_copy = high_resolution_clock::now();
      ADG copy = copy_ADG(adg);
      auto end_graph_copy = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_copy - start_graph_copy);


      /* Following is the two branches. */

      {
        // Forward
        // Fix the edge
        bool pruned;
        if (!use_grouping) {
          auto fixed_edge=fix_switchable_edge(graph, maxI, maxJ, false);
          auto start_dfs = high_resolution_clock::now();
          pruned = check_cycle_dfs(graph, fixed_edge.first);
          auto end_dfs = high_resolution_clock::now();
          dfsT += duration_cast<microseconds>(end_dfs - start_dfs);
        } else {
          // get the group of edges
          std::vector<std::pair<int,int> > edges= group_manager->get_groupable_edges(maxI, maxJ);
          auto fixed_edges=fix_switchable_edges(graph, edges, false);
          auto start_dfs = high_resolution_clock::now();
          std::vector<int> out_states;
          std::for_each(begin(fixed_edges), end(fixed_edges), [&out_states](std::pair<int,int> & edge) {
              out_states.emplace_back(edge.first);
          });
          pruned = check_cycle_dfs(graph,out_states);
          auto end_dfs = high_resolution_clock::now();
          dfsT += duration_cast<microseconds>(end_dfs - start_dfs);
        }

        if (pruned) { // Prune node
          pruned_node_cnt += 1;
        } else {
          shared_ptr<vector<int> > newts_tv_init;
          shared_ptr<vector<int> > newts_vt_init;
          sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

          shared_ptr<vector<int> > newts_tv;
          shared_ptr<vector<int> > newts_vt;
          shared_ptr<vector<int> > node_values = make_shared<vector<int> >(get<3>(graph), 0);

          auto start_sort = high_resolution_clock::now();
          tie(newts_tv, newts_vt) = topologicalSort(graph, newInitResult, currents, -1, -1);
          auto end_sort = high_resolution_clock::now();
          float val = heuristic_graph(adg, newts_tv, node_values);
          auto end_heuristic = high_resolution_clock::now();

          sortT += duration_cast<microseconds>(end_sort - start_sort);
          heuristicT += duration_cast<microseconds>(end_heuristic - end_sort);

          if (val>=0) {
            auto forward_node = std::make_shared<Node>(adg, val, node_values);
            auto start_pq_push = high_resolution_clock::now();
            pq.push(forward_node);
            auto end_pq_push = high_resolution_clock::now();
            pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

            added_node_cnt += 1;
          }
        }
      }

      {
        // Backward 
        Graph &graph = get<0>(copy);

        bool pruned;
        if (!use_grouping) {
          auto fixed_edge=fix_switchable_edge(graph, maxI, maxJ, true);
          auto start_dfs = high_resolution_clock::now();
          pruned = check_cycle_dfs(graph, fixed_edge.first);
          auto end_dfs = high_resolution_clock::now();
          dfsT += duration_cast<microseconds>(end_dfs - start_dfs);
        } else {
          // get the group of edges
          std::vector<std::pair<int,int> > edges= group_manager->get_groupable_edges(maxI, maxJ);
          auto fixed_edges=fix_switchable_edges(graph, edges, true);
          auto start_dfs = high_resolution_clock::now();
          std::vector<int> out_states;
          std::for_each(begin(fixed_edges), end(fixed_edges), [&out_states](std::pair<int,int> & edge) {
              out_states.emplace_back(edge.first);
          });
          pruned = check_cycle_dfs(graph,out_states);
          auto end_dfs = high_resolution_clock::now();
          dfsT += duration_cast<microseconds>(end_dfs - start_dfs);
        }

        if (pruned) { // Prune node
          pruned_node_cnt += 1;
        } else {
          shared_ptr<vector<int> > newts_tv_init;
          shared_ptr<vector<int> > newts_vt_init;
          sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

          shared_ptr<vector<int> > newts_tv;
          shared_ptr<vector<int> > newts_vt;
          shared_ptr<vector<int> > node_values = make_shared<vector<int> >(get<3>(graph), 0);

          auto start_sort = high_resolution_clock::now();
          tie(newts_tv, newts_vt) = topologicalSort(graph, newInitResult, currents, -1, -1);
          auto end_sort = high_resolution_clock::now();
          float val = heuristic_graph(copy, newts_tv, node_values);
          auto end_heuristic = high_resolution_clock::now();

          sortT += duration_cast<microseconds>(end_sort - start_sort);
          heuristicT += duration_cast<microseconds>(end_heuristic - end_sort);

          if (val>=0) {
            auto backward_node = std::make_shared<Node>(copy, val, node_values);

            auto start_pq_push = high_resolution_clock::now();
            pq.push(backward_node);
            auto end_pq_push = high_resolution_clock::now();
            pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

            added_node_cnt += 1;
          }
        }
      }
    }
  }

  // if nothing found, return the initial ADG
  return init_adg;
  // throw invalid_argument("no solution found");
}

ADG Astar::startExplore(ADG & adg, float cost, int input_sw_cnt, vector<int> & states) {
  init_adg=copy_ADG(adg);
  init_cost=cost;
  
  // TODO(rivers): if use grouping
  if (use_grouping) {
    // std::cout<<"input_sw_cnt: "<<input_sw_cnt<<std::endl;
    auto start = high_resolution_clock::now();
    group_manager=std::make_shared<GroupManager>(adg, states);
    auto end = high_resolution_clock::now();
    groupingT += duration_cast<microseconds>(end - start);
    // std::cout<<"group size: "<<group_manager->groups.size()<<std::endl;
   // group_manager->print_groups();
  }

  vertex_cnt = get<3>(get<0>(adg));
  sw_edge_cnt = input_sw_cnt;
  // std::cout << "vertex_cnt = " << vertex_cnt << ", sw_edge_cnt = " << sw_edge_cnt << "\n";

  /* Graph-Based Search */
  agentCnt = get_agentCnt(adg);
  Graph &graph = get<0>(adg);

  for (int agent = 0; agent < agentCnt; agent ++) {
    // get current global vertex idx
    int current = compute_vertex((*get<2>(adg)), agent, 0);
    currents.push_back(current);
  }
  
  // ts: topological sort, tv: topological order (time) -> vertex global idx
  shared_ptr<vector<int> > ts_tv_init;
  // ts: topological sort, vt: vertex global idx -> topological order (time)
  shared_ptr<vector<int> > ts_vt_init;
  sortResult initResult = make_pair(ts_tv_init, ts_vt_init);

  auto start_sort = high_resolution_clock::now();
  sortResult result = topologicalSort(graph, initResult, currents, -1, -1);
  auto end_sort = high_resolution_clock::now();
  sortT += duration_cast<microseconds>(end_sort - start_sort);

  // initialize the array for longest path length from any current vertices to an agent's goal vertex.
  auto node_values = make_shared<vector<int> >(get<3>(graph), 0);
  auto start_heuristic = high_resolution_clock::now();
  // compute the longest path length from any current vertices to an agent's goal vertex.
  float val = heuristic_graph(adg, result.first, node_values);
  auto end_heuristic = high_resolution_clock::now();
  heuristicT += duration_cast<microseconds>(end_heuristic - start_heuristic);

  if (val>=0) {
    auto root = make_shared<Node>(adg, val, node_values);

    auto start_pq_push = high_resolution_clock::now();
    pq.push(root);
    auto end_pq_push = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);
  }

  // expand the node
  return exploreNode();
}

