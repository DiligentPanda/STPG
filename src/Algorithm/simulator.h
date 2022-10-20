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
    bool move(vector<int>& moved, int agent, int* timeSpent, bool switchCheck);
    // Return the first switchable edge detected for the next step
    tuple<int, int, int, int> detectSwitch();
    

    int astep(int p, int d, int *delayer);
    bool amove(vector<int>& moved, int agent, int *timeSpent, int *delayer, int p, int d);
    int asimulate(int p, int d, ofstream &outFile);
};
#endif