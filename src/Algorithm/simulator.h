#ifndef SIMULATOR
#define SIMULATOR

#include "../ADG/generate_ADG.h"

class Simulator {
  public:         
    ADG adg;
    int* config;
    
    Simulator(ADG adg);
    int Step();
    bool Move(int* moved, int agent);
};
#endif