#ifndef SIMULATOR
#define SIMULATOR

#include "../ADG/generate_ADG.h"

class Simulator {
  public:         
    ADG adg;
    int* config;
    
    Simulator(char *fileName);
    Step();
};
#endif