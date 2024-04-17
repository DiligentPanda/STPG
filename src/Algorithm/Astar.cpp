#include "Algorithm/Astar.h"
#include <algorithm>
#include <limits.h>
#include "group/group.h"
#include "Algorithm/graph.h"

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
    double _w_astar,
    double _w_focal,
    uint random_seed
  ): rng(random_seed), w_astar(_w_astar), w_focal(_w_focal) {
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

int Astar::compute_partial_cost(const shared_ptr<Graph> & adg) {
  // Graph &graph = get<0>(copy);
  // remove_all_switchable_edges(graph);

  Simulator simulator(adg);
  int cost = simulator.print_soln();
  return cost;
}

// compute the longest path length from any current vertices to an agent's goal vertex.
// the heuristic is the sum of the longest path length.
double Astar::heuristic_graph(
    const shared_ptr<Graph> & adg, 
    const shared_ptr<vector<int> > & ts, 
    const shared_ptr<vector<int> > & values
  ) {

  for (int i: (*ts)) {
    int prevVal = values->at(i);
    auto && outNeib = adg->get_out_neighbor_global_ids(i);
    for (auto it = outNeib.begin(); it != outNeib.end(); it++) {
      int j = *it;
      int weight = 1;
      if (values->at(j) < prevVal + weight) values->at(j) = prevVal + weight;
    }
  }
  double sum = 0;
  for (int agent = 0; agent < agentCnt; agent ++) {
    int goalVert = (*adg->accum_state_cnts_end)[agent]-1;
    sum += values->at(goalVert);
  }

  // int partial_cost = compute_partial_cost(adg);
  // if (partial_cost != sum) {
  //   std::cout << "mismatched! PC = " << partial_cost << ", SUM = " << sum << "\n";
  // }
  return sum;
}

tuple<int, int, int> Astar::branch(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values) {
  int maxDiff = -1;
  int maxI = -1;
  int maxJ = -1;

  if (branch_order==BranchOrder::DEFAULT) {
    for (int i = 0; i < adg->get_num_states(); ++i) {
      int iTime = values->at(i);
      auto &outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
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
    for (int i = 0; i < adg->get_num_states(); ++i) {
      int iTime = values->at(i);
      auto &outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
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
    for (int i = 0; i < adg->get_num_states(); ++i) {
      int iTime = values->at(i);
      auto &outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
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
    for (int i = 0; i < adg->get_num_states(); ++i) {
      int iTime = values->at(i);
      auto &outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
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

tuple<int, int, int> Astar::enhanced_branch(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values) {
  int maxDiff = -1;
  int maxI = -1;
  int maxJ = -1;
  for (int i = 0; i < adg->get_num_states(); ++i) {
    int iTime = values->at(i);
    auto &outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
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

bool Astar::terminated(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values) {
  if (early_termination) {
    for (int i = 0; i < adg->get_num_states(); ++i) {
      int iTime = values->at(i);
      auto &outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
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
    for (int i = 0; i < adg->get_num_states(); ++i) {
      int iTime = values->at(i);
      auto &outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
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
    double group_size_max=0;
    double group_size_avg=0;
    double group_size_min=sw_edge_cnt;
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

int Astar::count_double_conflicting_edge_groups(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values) {
  set<int> double_conflicting_edge_groups;
  int cnt=0;
  for (int i = 0; i < adg->get_num_states(); ++i) {
    int iTime = values->at(i);
    auto & outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
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
        if (use_grouping) {
          int group_id=group_manager->get_group_id(i,j);
          if (group_id<0) {
            std::cout<<"edge doesn't have a group"<<std::endl;
            exit(19);
          } else {
            double_conflicting_edge_groups.insert(group_id);
          }
        } else {
          cnt+=1;
        }
      }
    }
  }
  if (use_grouping) {
    cnt=double_conflicting_edge_groups.size();
  }
  return cnt;
}


// adg in parent_node might be changed by forward node, so don't use it.
void Astar::add_node(const shared_ptr<Graph> & adg, const shared_ptr<SearchNode> & parent_node, vector<std::pair<int,int> > & fixed_edges) {

  auto start_sort = high_resolution_clock::now();
  shared_ptr<vector<int> > newts_tv_init;
  shared_ptr<vector<int> > newts_vt_init;
  sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);
  shared_ptr<vector<int> > newts_tv;
  shared_ptr<vector<int> > newts_vt;
  std::tie(newts_tv, newts_vt) = topologicalSort(*adg, newInitResult, currents, -1, -1);

  auto end_sort = high_resolution_clock::now();
  sortT += duration_cast<microseconds>(end_sort - start_sort);

  auto start_heuristic = high_resolution_clock::now();
  // initialize the array for longest path length from any current vertices to an agent's goal vertex.
  // auto node_values = make_shared<vector<int> >(parent_node->longest_path_lengths);
  // compute the longest path length from any current vertices to an agent's goal vertex.
  // auto g = heuristic_graph(adg, newts_tv, node_values);
  auto longest_path_lengths = compute_longest_paths(parent_node->longest_path_lengths, adg, fixed_edges);
  double g = 0;
  for (int agent_id=0;agent_id<adg->get_num_agents();++agent_id) {
    int goal_state=adg->get_global_state_id(agent_id, adg->get_num_states(agent_id)-1);
    g+=longest_path_lengths->at(goal_state);
  }
  auto end_heuristic = high_resolution_clock::now();
  heuristicT += duration_cast<microseconds>(end_heuristic - start_heuristic);

  auto start = high_resolution_clock::now();
  auto reverse_longest_path_lengths=compute_reverse_longest_paths(parent_node->reverse_longest_path_lengths, longest_path_lengths, adg, fixed_edges);

  auto h= heuristic_manager->computeInformedHeuristics(adg, *longest_path_lengths, *reverse_longest_path_lengths, 0);
  auto end = high_resolution_clock::now();
  extraHeuristicT += duration_cast<microseconds>(end - start);

  if (g+h<init_cost) {
    int num_sw=parent_node->num_sw-1;
    // int num_sw=count_double_conflicting_edge_groups(graph, node_values);
    auto child_node = std::make_shared<SearchNode>(adg, g, h*w_astar, longest_path_lengths, reverse_longest_path_lengths, num_sw);
    auto start_pq_push = high_resolution_clock::now();
    open_list->push(child_node);
    auto end_pq_push = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

    added_node_cnt += 1;
  }
}

void Astar::reverse_nonSwitchable_edges_basedOn_LongestPathValues(
  const shared_ptr<Graph> & adg, 
  const shared_ptr<vector<int> > & values
) {
  auto & times=*values;

  std::vector<std::pair<int,int> > need_to_reverse;


  for (int i = 0; i < adg->get_num_states(); ++i) {
    auto &outNeib = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
    for (auto it : outNeib) {
      int j = it;
      int time_i=times[i];
      int time_j=times[j];

      int back_i=j+1;
      int back_j=i-1;
      int time_back_i=times[back_i];
      int time_back_j=times[back_j];

      if (time_i>=time_j) {
          if (time_back_i>=time_back_j) {
              std::cout<<"error in set_switchable_nonSwitchable_basedOn_LongestPath: conflict not solved yet!"<<std::endl;
              std::cout<<"time_i="<<time_i<<" time_j="<<time_j<<std::endl;
              std::cout<<"time_back_i="<<time_back_i<<" time_back_j="<<time_back_j<<std::endl;
              exit(200);
          }
          // need to reverse
          need_to_reverse.emplace_back(i,j);
      }
    }
  }

  adg->fix_switchable_type2_edges(need_to_reverse, true);

}


shared_ptr<Graph> Astar::exploreNode() {
  auto start = high_resolution_clock::now();
  while (open_list->size() > 0) {
    explored_node_cnt += 1;

    if (explored_node_cnt % 1000000 == 0) {
      std::cout << explored_node_cnt << "\n";
    }

    auto start_pq_pop = high_resolution_clock::now();
    auto node = open_list->pop();
    auto end_pq_pop = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
    
    auto & adg = node->adg;
    auto & values = node->longest_path_lengths;

    int maxDiff, maxI, maxJ;
    auto start_branch = high_resolution_clock::now();
    // maxI, maxJ returns a type 2 edge to branch (maxDiff is useless for now)
    if (branch_order==BranchOrder::CONFLICT) {
      std::tie(maxDiff, maxI, maxJ) = enhanced_branch(adg, values);  
    } else {
      std::tie(maxDiff, maxI, maxJ) = branch(adg, values);
    }
    auto end_branch = high_resolution_clock::now();
    branchT += duration_cast<microseconds>(end_branch - start_branch);

    bool terminate = terminated(adg, values);

    auto end_term = high_resolution_clock::now();
    termT += duration_cast<microseconds>(end_term - end_branch);

    if (terminate || ((duration_cast<seconds>(end_branch - start)).count() >= timeout)) {

      // free all the search node in the priority queue.
      while (open_list->size() > 0) {
        auto node=open_list->pop();
        
        auto start_graph_free = high_resolution_clock::now();
        // we explicitly reset to count the time
        node.reset();
        auto end_graph_free = high_resolution_clock::now();
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
      }

      auto res=node->adg;

     // return the current ADG by fix switchable edges to non-switchable.
      if (terminate && early_termination) {
        reverse_nonSwitchable_edges_basedOn_LongestPathValues(adg, values);
      }

      adg->fix_all_switchable_type2_edges();

      auto start_graph_free = high_resolution_clock::now();
      // we explicitly reset to count the time
      node.reset();
      auto end_graph_free = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);

      return res;
    } else {
      auto start_graph_copy = high_resolution_clock::now();
      int agent1 = adg->get_agent_id(maxI);
      int agent2 = adg->get_agent_id(maxJ);
      shared_ptr<Graph> copy1 = adg->copy(agent1, agent2);
      shared_ptr<Graph> copy2 = adg->copy(agent1, agent2);
      auto end_graph_copy = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_copy - start_graph_copy);

      std::array<shared_ptr<Graph>, 2> branch_adgs { copy1,copy2 };
      // TODO(rivers): this order may affect the focal search, because we don't have rules to break tie if g,h,num_sw are all the same.
      // maybe we should break tie in the way that the edge should make less change (increase) to the longest distance of any nodes. In this way, the overall costs are less likely to increase.
      // Then since the edge to be branched voilates its original direction, the best one is to keep the reverse direction?
      std::array<bool, 2> branch_reversing_flags { true, false };

      /* Following is the two branches. */
      for (int i=0;i<2;++i) {
        auto & child_adg=branch_adgs[i];
        auto branch_reversing_flag=branch_reversing_flags[i];

        // Fix the edge
        bool pruned;
        vector<std::pair<int,int> > fixed_edges;
        if (!use_grouping) {
          auto fixed_edge=child_adg->fix_switchable_type2_edge(maxI, maxJ, branch_reversing_flag, true);
          fixed_edges.emplace_back(fixed_edge);
          auto start_dfs = high_resolution_clock::now();
          pruned = check_cycle_dfs(*child_adg, fixed_edge.first);
          auto end_dfs = high_resolution_clock::now();
          dfsT += duration_cast<microseconds>(end_dfs - start_dfs);
        } else {
          // get the group of edges
          std::vector<std::pair<int,int> > edges= group_manager->get_groupable_edges(maxI, maxJ);
          fixed_edges=child_adg->fix_switchable_type2_edges(edges, branch_reversing_flag, true);
          auto start_dfs = high_resolution_clock::now();
          std::vector<int> out_states;
          std::for_each(std::begin(fixed_edges), std::end(fixed_edges), [&out_states](std::pair<int,int> & edge) {
              out_states.emplace_back(edge.first);
          });
          pruned = check_cycle_dfs(*child_adg,out_states);
          auto end_dfs = high_resolution_clock::now();
          dfsT += duration_cast<microseconds>(end_dfs - start_dfs);
        }

        if (pruned) { // Prune node
          pruned_node_cnt += 1;
        } else {
          add_node(child_adg, node, fixed_edges);
        }
      }

      auto start_graph_free = high_resolution_clock::now();
      // we explicitly reset to count the time
      node.reset();
      auto end_graph_free = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);

    }
  }

  // if nothing found, return the initial ADG
  return init_adg;
  // throw invalid_argument("no solution found");
}

shared_ptr<Graph> Astar::startExplore(const shared_ptr<Graph> & adg, double cost, vector<int> & states) {
  auto start_graph_free = high_resolution_clock::now();
  init_adg=adg->copy();
  auto end_graph_free = high_resolution_clock::now();
  copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
  
  init_cost=cost;

  open_list=std::make_shared<OpenList>(w_focal);
  
  vertex_cnt = adg->get_num_states();
  sw_edge_cnt = adg->get_num_switchable_edges();
  agentCnt = adg->get_num_agents();
  std::cout << "vertex_cnt = " << vertex_cnt << ", sw_edge_cnt = " << sw_edge_cnt << "\n";

  // TODO(rivers): if use grouping
  if (use_grouping) {
    // std::cout<<"input_sw_cnt: "<<input_sw_cnt<<std::endl;
    auto start = high_resolution_clock::now();
    group_manager=std::make_shared<GroupManager>(init_adg, states);
    auto end = high_resolution_clock::now();
    groupingT += duration_cast<microseconds>(end - start);
    std::cout<<"group size: "<<group_manager->groups.size()<<std::endl;
   // group_manager->print_groups();
  }

  /* Graph-Based Search */
  // BUG(rivers): why we start current at 0, would it be a bug? shouldn't we start with current states?
  for (int agent = 0; agent < agentCnt; agent ++) {
    // get current global vertex idx
    int global_state_id = adg->get_global_state_id(agent, 0);
    currents.push_back(global_state_id);
  }

  auto fake_parent=std::make_shared<SearchNode>(0);
  if (use_grouping) {
    fake_parent->num_sw=group_manager->groups.size();
  } else {
    fake_parent->num_sw=sw_edge_cnt;
  }
  fake_parent->longest_path_lengths=make_shared<vector<int> >(adg->get_num_states(), 0);
  fake_parent->reverse_longest_path_lengths=make_shared<vector<map<int,int> > >(adg->get_num_states());
  fake_parent->num_sw=sw_edge_cnt;
  vector<pair<int,int> > fixed_edges;
  add_node(adg, fake_parent, fixed_edges);

  // expand the node
  return exploreNode();
}

