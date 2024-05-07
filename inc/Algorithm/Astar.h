#ifndef ASTAR
#define ASTAR

#include <queue>
#include <chrono>
using namespace std::chrono;

#include "simulator.h"
#include "nlohmann/json.hpp"
#include "group/group.h"
#include <memory>
#include <vector>
#include "Algorithm/heuristic.h"
#include "Algorithm/SearchNode.h"
#include "Algorithm/graph.h"
#include "OpenList.h"
#include "solver.h"
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
      double _w_astar=1.0,
      double _w_focal=1.0,
      uint random_seed=0
    );
    shared_ptr<Graph> solve(const shared_ptr<Graph> & adg, double cost, vector<int> & states);
    double heuristic_graph(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & topological_sort_order, const shared_ptr<vector<int> > & longest_path_lengths);

    int compute_partial_cost(const shared_ptr<Graph> & adg);

    void write_stats(nlohmann::json & stats);

  private:
    int calcTime(Simulator simulator);
    shared_ptr<Graph> exploreNode();
    tuple<int, int, int> enhanced_branch(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);
    tuple<int, int, int> branch(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);
    bool terminated(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);
    int count_double_conflicting_edge_groups(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);
    void reverse_nonSwitchable_edges_basedOn_LongestPathValues(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);

    void add_node(const shared_ptr<Graph> & adg, const shared_ptr<SearchNode> & parent_node, vector<std::pair<int,int> > & fixed_edges);

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

    vector<double> open_list_min_f_vals;
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

    double w_astar = 1.0;
    double w_focal = 1.0;

    shared_ptr<Graph> init_adg;
    double init_cost;
};
#endif