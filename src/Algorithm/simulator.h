#ifndef SIMULATOR
#define SIMULATOR

#include "../ADG/generate_ADG.h"

class Simulator {
  public:         
    ADG adg;
    int* states;
    
    Simulator(ADG adg);
    Simulator(ADG adg, int *visited_states);
    int step();
    bool move(int* moved, int agent, int* timeSpent);
    // Return the first switchable edge detected for the next step
    pair<pair<int, int>, pair<int, int>> detectSwitch();
};
#endif