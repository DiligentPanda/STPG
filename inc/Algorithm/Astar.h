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
#include <random>

enum BranchOrder {
  DEFAULT,
  CONFLICT,
  LARGEST_DIFF,
  RANDOM,
  EARLIEST
};

class Astar {
  public:
    Astar();
    Astar(int input_timeout);
    Astar(
      int input_timeout, 
      bool input_fast_version,
      const string & branch_order="default", 
      bool use_grouping=false, 
      const string & _heuristic="zero", 
      const bool early_termination=false,
      double _w_astar=1.0,
      double _w_focal=1.0,
      uint random_seed=0
    );
    shared_ptr<Graph> startExplore(const shared_ptr<Graph> & adg, double cost, vector<int> & states);
    double heuristic_graph(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & topological_sort_order, const shared_ptr<vector<int> > & longest_path_lengths);

    int compute_partial_cost(const shared_ptr<Graph> & adg);

    void print_stats();
    void print_stats(ofstream &outFile);
    void print_stats(nlohmann::json & stats);

  private:
    int calcTime(Simulator simulator);
    shared_ptr<Graph> exploreNode();
    tuple<int, int, int> enhanced_branch(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);
    tuple<int, int, int> branch(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);
    bool terminated(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);
    int count_double_conflicting_edge_groups(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);
    void reverse_nonSwitchable_edges_basedOn_LongestPathValues(const shared_ptr<Graph> & adg, const shared_ptr<vector<int> > & values);

    void add_node(const shared_ptr<Graph> & adg, const shared_ptr<SearchNode> & parent_node);

    microseconds extraHeuristicT = std::chrono::microseconds::zero();
    microseconds groupingT = std::chrono::microseconds::zero();
    microseconds heuristicT = std::chrono::microseconds::zero();
    microseconds branchT = std::chrono::microseconds::zero();
    microseconds sortT = std::chrono::microseconds::zero();
    microseconds pqT = std::chrono::microseconds::zero();
    microseconds copy_free_graphsT = std::chrono::microseconds::zero();
    microseconds dfsT = std::chrono::microseconds::zero();
    microseconds termT = std::chrono::microseconds::zero();

    microseconds totalT  = std::chrono::seconds::zero();

    int explored_node_cnt = 0;
    int pruned_node_cnt = 0;
    int added_node_cnt = 0;

    int vertex_cnt = 0;
    int sw_edge_cnt = 0;
    
    int timeout = 300;

    vector<int> currents;
    shared_ptr<OpenList> open_list;
    int agentCnt = 0;

    bool fast_version = false;
    BranchOrder branch_order=BranchOrder::DEFAULT;  
    mt19937 rng;

    bool use_grouping=false;
    shared_ptr<GroupManager> group_manager;
    shared_ptr<HeuristicManager> heuristic_manager;

    bool early_termination = false;

    double w_astar = 1.0;
    double w_focal = 1.0;

    shared_ptr<Graph> init_adg;
    double init_cost;
};
#endif