#ifndef SIMULATOR
#define SIMULATOR

#include "graph/graph.h"

class Simulator {
  public:         
    shared_ptr<Graph> adg;
    // how many states are visited by each agent. used to describe the execution state.
    vector<int> states;
    
    Simulator(const shared_ptr<Graph> & adg);
    Simulator(const shared_ptr<Graph> & adg, vector<int> visited_states);
    int step(bool switchCheck=true);
    int checkMovable(vector<int>& movable, bool switchCheck);
    void print_location(ofstream &outFile, Location location);
    int print_soln(const char* outFileName);
    int print_soln();

};
#endif