#ifndef SIMULATOR
#define SIMULATOR

#include "../ADG/generate_ADG.h"

class Simulator {
  public:         
    ADG adg;
    int* states;
    
    Simulator(ADG adg);
    Simulator(ADG adg, int *visited_states);
    int step(bool switcCheck);
    bool move(int* moved, int agent, int* timeSpent, bool switchCheck);
    // Return the first switchable edge detected for the next step
    tuple<int, int, int, int> detectSwitch();
};
#endif