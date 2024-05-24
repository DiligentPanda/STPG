#include "Algorithm/Astar.h"
#include <algorithm>
#include "group/group.h"
#include "Algorithm/graph_algo.h"
#include <iomanip>   

Astar::Astar() {
}

Astar::Astar(int input_timeout) {
  timeout = input_timeout;
}

Astar::Astar(
    int input_timeout, 
    bool input_fast_version, 
    const string & _branch_order, 
    const string & _grouping_method, 
    const string & _heuristic, 
    bool early_termination,
    bool incremental, 
    COST_TYPE _w_astar,
    COST_TYPE _w_focal,
    uint random_seed
  ): rng(random_seed), incremental(incremental), w_astar(_w_astar), w_focal(_w_focal) {
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
  } else if (_heuristic=="fast_wcg_greedy") {
    heuristic=HeuristicType::FAST_WCG_GREEDY;
  } else {
    std::cout<<"unknown heuristic: "<<_heuristic<<std::endl;
    exit(18);
  }

  if (_grouping_method=="none") {
    use_grouping=false;
    grouping_method=GroupingMethod::NONE;
  } else if (_grouping_method=="simple") {
    use_grouping=true;
    grouping_method=GroupingMethod::SIMPLE;
  } else if (_grouping_method=="simple_merge") {
    use_grouping=true;
    grouping_method=GroupingMethod::SIMPLE_MERGE;
  } else if (_grouping_method=="all") {
    use_grouping=true;
    grouping_method=GroupingMethod::ALL;
  } else {
    std::cout<<"unknown grouping method: "<<_grouping_method<<std::endl;
    exit(19);
  }


  this->early_termination=early_termination;
  this->heuristic_manager=std::make_shared<HeuristicManager>(heuristic);
}

tuple<int, int, COST_TYPE> Astar::branch(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values) {
  COST_TYPE maxDiff = -1;
  int maxI = -1;
  int maxJ = -1;

  if (branch_order==BranchOrder::DEFAULT) {
    for (int i = 0; i < graph->get_num_states(); ++i) {
      COST_TYPE iTime = values->at(i);
      auto &outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
      for (auto it : outNeib) {
        int j = it;
        COST_TYPE jTime = values->at(j);
        COST_TYPE diff = iTime - jTime;
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
    for (int i = 0; i < graph->get_num_states(); ++i) {
      COST_TYPE iTime = values->at(i);
      auto &outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
      for (auto it : outNeib) {
        int j = it;
        COST_TYPE jTime = values->at(j);
        COST_TYPE diff = iTime - jTime;
        if (diff > maxDiff) {
          maxDiff = diff;
          maxI = i;
          maxJ = j;
        }
      }
    }
  } else if (branch_order==BranchOrder::EARLIEST) {
    COST_TYPE earliest_t=COST_MAX;
    for (int i = 0; i < graph->get_num_states(); ++i) {
      COST_TYPE iTime = values->at(i);
      auto &outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
      for (auto it : outNeib) {
        int j = it;
        COST_TYPE jTime = values->at(j);
        COST_TYPE diff = iTime - jTime;
        if (diff>=0 && jTime<earliest_t) {
          maxDiff = diff;
          maxI = i;
          maxJ = j;
          earliest_t=jTime;
        }
      }
    }
  } else if (branch_order==BranchOrder::RANDOM) {

    std::vector<std::tuple<int,int,COST_TYPE> > tuples;
    for (int i = 0; i < graph->get_num_states(); ++i) {
      COST_TYPE iTime = values->at(i);
      auto &outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
      for (auto it : outNeib) {
        int j = it;
        COST_TYPE jTime = values->at(j);
        COST_TYPE diff = iTime - jTime;
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

tuple<int, int, COST_TYPE> Astar::enhanced_branch(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values) {
  COST_TYPE maxDiff = -1;
  int maxI = -1;
  int maxJ = -1;
  for (int i = 0; i < graph->get_num_states(); ++i) {
    COST_TYPE iTime = values->at(i);
    auto &outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
    for (auto it : outNeib) {
      int j = it;
      int backI = j+1;
      int backJ = i-1;
      COST_TYPE jTime = values->at(j);
      COST_TYPE backITime = values->at(backI);
      COST_TYPE backJTime = values->at(backJ);
      COST_TYPE diff = iTime - jTime;
      COST_TYPE backDiff = backITime - backJTime;

      // TODO(rivers): should we try to find maxDiff
      if (diff>=0 && backDiff>=0) {
        COST_TYPE _diff = std::min(diff,backDiff);
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

bool Astar::terminated(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values) {
  if (early_termination) {
    for (int i = 0; i < graph->get_num_states(); ++i) {
      COST_TYPE iTime = values->at(i);
      auto &outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
      for (auto it : outNeib) {
        int j = it;
        int backI = j+1;
        int backJ = i-1;
        COST_TYPE jTime = values->at(j);
        COST_TYPE backITime = values->at(backI);
        COST_TYPE backJTime = values->at(backJ);
        COST_TYPE diff = iTime - jTime;
        COST_TYPE backDiff = backITime - backJTime;

        if (diff>=0 && backDiff>=0) {
          return false;
        }
      }
    }
    return true;
  } else {
    for (int i = 0; i < graph->get_num_states(); ++i) {
      COST_TYPE iTime = values->at(i);
      auto &outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
      for (auto it : outNeib) {
        int j = it;
        COST_TYPE jTime = values->at(j);
        COST_TYPE diff = iTime - jTime;
        if (diff >= 0) {
          return false;
        }
      }
    }
    return true;
  }
}

void Astar::write_stats(nlohmann::json & stats) {
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
  stats["search_time"]=searchT.count();
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
  // stats["open_list_f_min_vals"]=open_list_min_f_vals;
  // stats["selected_edges"]=selected_edges;
}

  auto start_graph_free = high_resolution_clock::now();
int Astar::count_double_conflicting_edge_groups(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values) {
  set<int> COST_TYPE_conflicting_edge_groups;
  int cnt=0;
  for (int i = 0; i < graph->get_num_states(); ++i) {
    int iTime = values->at(i);
    auto & outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
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
            COST_TYPE_conflicting_edge_groups.insert(group_id);
          }
        } else {
          cnt+=1;
        }
      }
    }
  }
  if (use_grouping) {
    cnt=COST_TYPE_conflicting_edge_groups.size();
  }
  return cnt;
}

// NOTE: this function don't consider switchable edges, that's why partial
// if want get full execution time, please fixed all switchable edges first.
COST_TYPE Astar::compute_partial_execution_time(const shared_ptr<Graph> & graph) {
  shared_ptr<vector<COST_TYPE> > fake_ptr(nullptr);
  vector<std::pair<int,int> > fake_edges;
  auto longest_path_lengths = compute_longest_paths(fake_ptr, graph, fake_edges, false);

  COST_TYPE g = 0;
  for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
    int goal_state=graph->get_global_state_id(agent_id, graph->get_num_states(agent_id)-1);
    g+=longest_path_lengths->at(goal_state);
  }

  return g;
}


// graph in parent_node might be changed by forward node, so don't use it.
void Astar::add_node(const shared_ptr<Graph> & graph, const shared_ptr<SearchNode> & parent_node, vector<std::pair<int,int> > & fixed_edges) {

  auto start_sort = high_resolution_clock::now();
  // shared_ptr<vector<int> > newts_tv_init;
  // shared_ptr<vector<int> > newts_vt_init;
  // sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);
  // shared_ptr<vector<int> > newts_tv;
  // shared_ptr<vector<int> > newts_vt;
  // std::tie(newts_tv, newts_vt) = topologicalSort(*graph, newInitResult, currents, -1, -1);

  auto end_sort = high_resolution_clock::now();
  sortT += duration_cast<microseconds>(end_sort - start_sort);

  auto start_heuristic = high_resolution_clock::now();
  // initialize the array for longest path length from any current vertices to an agent's goal vertex.
  // auto node_values = make_shared<vector<int> >(parent_node->longest_path_lengths);
  // compute the longest path length from any current vertices to an agent's goal vertex.
  // auto g = heuristic_graph(graph, newts_tv, node_values);
  auto longest_path_lengths = compute_longest_paths(parent_node->longest_path_lengths, graph, fixed_edges, incremental);

  // auto longest_path_lengths2 = compute_longest_paths(parent_node->longest_path_lengths, graph, fixed_edges, false);

  // std::cout<<"node explored: "<<explored_node_cnt<<std::endl;
  // bool err=false;
  // for (int state=0;state<(int)graph->get_num_states();++state) {
  //   if (longest_path_lengths->at(state)!=longest_path_lengths2->at(state)) {
  //     auto p=graph->get_agent_state_id(state);
  //     std::cout<<"error in longest path length computation. agent: "<<p.first<<" state: "<<p.second<<" length: "<<longest_path_lengths->at(state)<<" vs "<<longest_path_lengths2->at(state)<<std::endl;
  //     err=true;
  //   }
  // }
  // if (err) {
  //   exit(200);
  // }


  COST_TYPE g = 0;
  for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
    int goal_state=graph->get_global_state_id(agent_id, graph->get_num_states(agent_id)-1);
    g+=longest_path_lengths->at(goal_state);
  }
  auto end_heuristic = high_resolution_clock::now();
  heuristicT += duration_cast<microseconds>(end_heuristic - start_heuristic);

  auto start = high_resolution_clock::now();
  // TODO(rivers): we can write a fast but approximate version.
  const bool fast_approximate=heuristic_manager->type==HeuristicType::FAST_WCG_GREEDY;
  COST_TYPE h;
  shared_ptr<vector<shared_ptr<map<int,COST_TYPE> > > > reverse_longest_path_lengths;
  if (heuristic_manager->type==HeuristicType::WCG_GREEDY) {
    reverse_longest_path_lengths=compute_reverse_longest_paths(parent_node->reverse_longest_path_lengths, longest_path_lengths, graph, fixed_edges, incremental);
  }
  h = heuristic_manager->computeInformedHeuristics(graph, longest_path_lengths, reverse_longest_path_lengths, 0, fast_approximate);

  // we cannot use the following for the root! A hidden bug.
  // the following max basically comes from the fact that the child's cost must be larger than the parent's because of the increasing number of edges.
  if (parent_node->longest_path_lengths!=nullptr) {
    h=max(h,parent_node->g+parent_node->h-g);
  }

  auto end = high_resolution_clock::now();
  extraHeuristicT += duration_cast<microseconds>(end - start);

  // TODO(rivers): we could also select the next edge to branch here.

  if (g+h<init_cost) {
    int num_sw=parent_node->num_sw-1;
    // int num_sw=count_COST_TYPE_conflicting_edge_groups(graph, node_values);

    shared_ptr<SearchNode> child_node;
    if (incremental) {
      child_node = std::make_shared<SearchNode>(graph, g, h*w_astar, longest_path_lengths, reverse_longest_path_lengths, num_sw, parent_node);
    } else {
      child_node = std::make_shared<SearchNode>(graph, g, h*w_astar, longest_path_lengths, nullptr, num_sw, parent_node);
    }

    auto start_pq_push = high_resolution_clock::now();
    open_list->push(child_node);
    auto end_pq_push = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

    added_node_cnt += 1;
  }
}

void Astar::reverse_nonSwitchable_edges_basedOn_LongestPathValues(
  const shared_ptr<Graph> & graph, 
  const shared_ptr<vector<COST_TYPE> > & values
) {
  auto & times=*values;

  std::vector<std::pair<int,int> > need_to_reverse;


  for (int i = 0; i < graph->get_num_states(); ++i) {
    auto &outNeib = graph->switchable_type2_edges->get_out_neighbor_global_ids(i);
    for (auto it : outNeib) {
      int j = it;
      COST_TYPE time_i=times[i];
      COST_TYPE time_j=times[j];

      int back_i=j+1;
      int back_j=i-1;
      COST_TYPE time_back_i=times[back_i];
      COST_TYPE time_back_j=times[back_j];

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

  graph->fix_switchable_type2_edges(need_to_reverse, true);

}


shared_ptr<Graph> Astar::exploreNode() {
  auto start = high_resolution_clock::now();
  while (open_list->size() > 0) {
    COST_TYPE f_min_val=open_list->top()->f;
    open_list_min_f_vals.push_back(f_min_val);

    explored_node_cnt += 1;

    if (explored_node_cnt % 1000000 == 0) {
      std::cout << explored_node_cnt << "\n";
    }

    auto start_pq_pop = high_resolution_clock::now();
    auto node = open_list->pop();
    auto end_pq_pop = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
    
    auto & graph = node->graph;
    auto & values = node->longest_path_lengths;

    int maxDiff, maxI, maxJ;
    auto start_branch = high_resolution_clock::now();
    // maxI, maxJ returns a type 2 edge to branch (maxDiff is useless for now)
    if (branch_order==BranchOrder::CONFLICT) {
      std::tie(maxDiff, maxI, maxJ) = enhanced_branch(graph, values);  
    } else {
      std::tie(maxDiff, maxI, maxJ) = branch(graph, values);
    }
    selected_edges.emplace_back(maxI,maxJ,maxDiff);
    auto end_branch = high_resolution_clock::now();
    branchT += duration_cast<microseconds>(end_branch - start_branch);

    bool terminate = terminated(graph, values);

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

      // timeout
      if (!terminate) {
        return init_graph;
      }

      std::cout<<"replanned graph's g-value: "<<(int)(node->g)<<std::endl;
      
      auto res=node->graph;

     // return the current ADG by fix switchable edges to non-switchable.
      if (terminate && early_termination) {
        reverse_nonSwitchable_edges_basedOn_LongestPathValues(res, values);
      }

      res->fix_all_switchable_type2_edges();

      auto start_graph_free = high_resolution_clock::now();
      // we explicitly reset to count the time
      node.reset();
      auto end_graph_free = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);

      return res;
    } else {
      auto start_graph_copy = high_resolution_clock::now();
      int agent1 = graph->get_agent_id(maxI);
      int agent2 = graph->get_agent_id(maxJ);
      shared_ptr<Graph> copy1 = graph->copy(agent1, agent2);
      shared_ptr<Graph> copy2 = graph->copy(agent1, agent2);
      auto end_graph_copy = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_copy - start_graph_copy);

      std::array<shared_ptr<Graph>, 2> branch_graphs { copy1,copy2 };
      // TODO(rivers): this order may affect the focal search, because we don't have rules to break tie if g,h,num_sw are all the same.
      // maybe we should break tie in the way that the edge should make less change (increase) to the longest distance of any nodes. In this way, the overall costs are less likely to increase.
      // Then since the edge to be branched voilates its original direction, the best one is to keep the reverse direction?
      std::array<bool, 2> branch_reversing_flags { true, false };

      /* Following is the two branches. */
      for (int i=0;i<2;++i) {
        auto & child_graph=branch_graphs[i];
        auto branch_reversing_flag=branch_reversing_flags[i];

        // Fix the edge
        bool pruned;
        vector<std::pair<int,int> > fixed_edges;
        if (!use_grouping) {
          auto fixed_edge=child_graph->fix_switchable_type2_edge(maxI, maxJ, branch_reversing_flag, true);
          fixed_edges.emplace_back(fixed_edge);
          auto start_dfs = high_resolution_clock::now();
          pruned = check_cycle_dfs(*child_graph, fixed_edge.first);
          auto end_dfs = high_resolution_clock::now();
          dfsT += duration_cast<microseconds>(end_dfs - start_dfs);
        } else {
          // get the group of edges
          std::vector<std::pair<int,int> > edges= group_manager->get_groupable_edges(maxI, maxJ);
          // we cannot check here unless we find all groups
          fixed_edges=child_graph->fix_switchable_type2_edges(edges, branch_reversing_flag, false);
          auto start_dfs = high_resolution_clock::now();
          std::vector<int> out_states;
          std::for_each(std::begin(fixed_edges), std::end(fixed_edges), [&out_states](std::pair<int,int> & edge) {
              out_states.emplace_back(edge.first);
          });
          pruned = check_cycle_dfs(*child_graph,out_states);
          auto end_dfs = high_resolution_clock::now();
          dfsT += duration_cast<microseconds>(end_dfs - start_dfs);
        }

        if (pruned) { // Prune node
          pruned_node_cnt += 1;
        } else {
          add_node(child_graph, node, fixed_edges);
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
  return init_graph;
  // throw invalid_argument("no solution found");
}

shared_ptr<Graph> Astar::solve(const shared_ptr<Graph> & _graph) {
  if (!_graph->is_fixed()) {
    std::cout<<"Astar:: graph is not fixed"<<std::endl;
    exit(100);
  }

  searchT = microseconds(timeout*1000000);
  auto start_search = high_resolution_clock::now();

  auto start_graph_free = high_resolution_clock::now();
  // since many operations are in-place, should never operate on the original graph. make a copy first.
  auto graph=_graph->copy();
  auto end_graph_free = high_resolution_clock::now();
  copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
  // make the graph switchable
  graph->make_switchable();

  // auto fixed_init_graph=init_graph->copy();
  // fixed_init_graph->fix_all_switchable_type2_edges();
  init_graph=_graph;
  init_cost=compute_partial_execution_time(init_graph);
  open_list=std::make_shared<OpenList>(w_focal);
  
  vertex_cnt = graph->get_num_states();
  sw_edge_cnt = graph->get_num_switchable_edges();
  agentCnt = graph->get_num_agents();
  // std::cout << "vertex_cnt = " << vertex_cnt << ", sw_edge_cnt = " << sw_edge_cnt << "\n";

  // TODO(rivers): if use grouping
  if (use_grouping) {
    // std::cout<<"input_sw_cnt: "<<input_sw_cnt<<std::endl;
    auto start = high_resolution_clock::now();
    group_manager=std::make_shared<GroupManager>(graph, *(graph->curr_states), grouping_method);
    auto end = high_resolution_clock::now();
    groupingT += duration_cast<microseconds>(end - start);
    std::cout<<"group size: "<<group_manager->groups.size()<<std::endl;
   // group_manager->print_groups();
  }

  /* Graph-Based Search */
  auto fake_parent=std::make_shared<SearchNode>(0);
  if (use_grouping) {
    fake_parent->num_sw=group_manager->groups.size();
  } else {
    fake_parent->num_sw=sw_edge_cnt;
  }

  fake_parent->longest_path_lengths=nullptr;
  fake_parent->reverse_longest_path_lengths=nullptr;
  
  fake_parent->num_sw=sw_edge_cnt;
  vector<pair<int,int> > fixed_edges;
  add_node(graph, fake_parent, fixed_edges);

  // expand the node
  auto && res_graph = exploreNode();

  auto end_search = high_resolution_clock::now();
  searchT=duration_cast<microseconds>(end_search - start_search);

  std::cout<<"search time: "<<searchT.count()/1000000.0<<std::endl;

  return res_graph;
}

