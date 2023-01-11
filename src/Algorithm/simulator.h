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

    int astep(int p, int d, int *delayer);
    bool amove(vector<int>& moved, int agent, int *timeSpent, int *delayer, int p, int d);
    int asimulate(int p, int d, ofstream &outFile);
};
#endif