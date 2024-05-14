#ifndef ASTAR
#define ASTAR

#include <queue>
#include <chrono>
using namespace std::chrono;

#include "nlohmann/json.hpp"
#include "group/group.h"
#include <memory>
#include <vector>
#include "Algorithm/heuristic.h"
#include "Algorithm/SearchNode.h"
#include "Algorithm/graph.h"
#include "OpenList.h"
#include "solver.h"
#include "define.h"
#include <random>

enum BranchOrder {
  DEFAULT,
  CONFLICT,
  LARGEST_DIFF,
  RANDOM,
  EARLIEST
};

class Astar: public Solver {
  public:
    Astar();
    Astar(int input_timeout);
    Astar(
      int input_timeout, 
      bool input_fast_version,
      const string & _branch_order="default", 
      const string & _grouping_method="none", 
      const string & _heuristic="zero", 
      bool early_termination=false,
      bool incremental=false,
      COST_TYPE _w_astar=1.0,
      COST_TYPE _w_focal=1.0,
      uint random_seed=0
    );
    shared_ptr<Graph> solve(const shared_ptr<Graph> & graph, COST_TYPE cost, vector<int> & states);
    void write_stats(nlohmann::json & stats);

  private:
    shared_ptr<Graph> exploreNode();
    tuple<int, int, COST_TYPE> enhanced_branch(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values);
    tuple<int, int, COST_TYPE> branch(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values);
    bool terminated(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values);
    int count_COST_TYPE_conflicting_edge_groups(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values);
    void reverse_nonSwitchable_edges_basedOn_LongestPathValues(const shared_ptr<Graph> & graph, const shared_ptr<vector<COST_TYPE> > & values);

    void add_node(const shared_ptr<Graph> & graph, const shared_ptr<SearchNode> & parent_node, vector<std::pair<int,int> > & fixed_edges);

    microseconds extraHeuristicT = std::chrono::microseconds::zero();
    microseconds groupingT = std::chrono::microseconds::zero();
    microseconds heuristicT = std::chrono::microseconds::zero();
    microseconds branchT = std::chrono::microseconds::zero();
    microseconds sortT = std::chrono::microseconds::zero();
    microseconds pqT = std::chrono::microseconds::zero();
    microseconds copy_free_graphsT = std::chrono::microseconds::zero();
    microseconds dfsT = std::chrono::microseconds::zero();
    microseconds termT = std::chrono::microseconds::zero();

    microseconds searchT  = std::chrono::seconds::zero();

    int explored_node_cnt = 0;
    int pruned_node_cnt = 0;
    int added_node_cnt = 0;

    vector<COST_TYPE> open_list_min_f_vals;
    vector<tuple<int,int,int> > selected_edges;

    int vertex_cnt = 0;
    int sw_edge_cnt = 0;
    
    int timeout = 300;

    vector<int> currents;
    shared_ptr<OpenList> open_list;
    int agentCnt = 0;

    // the fast version simply means to use graph-based heuristics, which is faster than the one based on the simulation.
    // TODO: I think we no longer support the simulation-based heuristics and we should remove this option.
    bool fast_version = false;
    BranchOrder branch_order=BranchOrder::DEFAULT;  
    mt19937 rng;

    bool use_grouping=false;
    GroupingMethod grouping_method=GroupingMethod::NONE;
    shared_ptr<GroupManager> group_manager;
    shared_ptr<HeuristicManager> heuristic_manager;

    bool early_termination = false;

    // for heuristic computation

    bool incremental = false;

    COST_TYPE w_astar = 1.0;
    COST_TYPE w_focal = 1.0;

    shared_ptr<Graph> init_graph;
    COST_TYPE init_cost;
};
#endif