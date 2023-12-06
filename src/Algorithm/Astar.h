#ifndef ASTAR
#define ASTAR

#include <queue>
#include <chrono>
using namespace std::chrono;

#include "../ADG/ADG_utilities.h"
#include "simulator.h"

class Astar {
  public:
    Astar();
    Astar(int input_timeout);
    Astar(int input_timeout, bool input_term_shortcut);
    ADG startExplore(ADG &adg);
    int heuristic_graph(ADG &adg, vector<int> *ts, vector<int> *values);

    void print_stats();
    void print_stats(ofstream &outFile);
    
  private:
    class Compare {
      public:
        bool operator() (Node* s1, Node* s2)
        {
          int val1 = get<1>(*s1);
          int val2 = get<1>(*s2);

          return val1 > val2;
        }
    };

    int calcTime(Simulator simulator);
    ADG exploreNode();
    tuple<int, int, int> branch(Graph &graph, vector<int> *values);

    microseconds heuristicT = std::chrono::microseconds::zero();
    microseconds branchT = std::chrono::microseconds::zero();
    microseconds sortT = std::chrono::microseconds::zero();
    microseconds pqT = std::chrono::microseconds::zero();
    microseconds copy_free_graphsT = std::chrono::microseconds::zero();
    microseconds dfsT = std::chrono::microseconds::zero();

    microseconds totalT  = std::chrono::seconds::zero();

    int explored_node_cnt = 0;
    int pruned_node_cnt = 0;
    int added_node_cnt = 0;
    
    int timeout = 300;

    vector<int> currents;
    priority_queue<Node*, vector<Node*>, Compare> pq;
    int agentCnt = 0;

    bool term_shortcut = false;
};
#endif