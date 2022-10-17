#include "simulator.h"

Simulator::Simulator(char *fileName) {
  adg = construct_ADG(fileName);
  
}