#ifndef SIMULATOR
#define SIMULATOR

#include "../ADG/generate_ADG.h"

class Simulator {
  public:         
    ADG adg;
    vector<int> states;
    
    Simulator(ADG adg);
    Simulator(ADG adg, vector<int> visited_states);
    int step(bool switchCheck);
    int checkMovable(vector<int>& movable);
    void print_location(ofstream &outFile, Location location);
    int print_soln(const char* outFileName);


    int step_wdelay(int p, bool *delay_mark, int *delayed_agent);
    // bool amove(vector<int>& moved, int agent, int *timeSpent, int *delayer, int p, int d);
    int simulate_wdelay(int p, int d, ofstream &outFile, int timeout, bool term_shortcut);
};
#endif