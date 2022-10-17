#ifndef SIMULATOR
#define SIMULATOR

#include "../ADG/generate_ADG.h"

class Simulator {
  public:         
    ADG adg;
    int* config;
    
    Simulator(ADG adg);
    int step();
    bool move(int* moved, int agent, int* timeSpent);
};
#endif