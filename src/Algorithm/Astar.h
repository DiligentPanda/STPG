#include <queue>
#include <chrono>
using namespace std::chrono;

#include "../ADG/ADG_utilities.h"
#include "simulator.h"

typedef tuple<ADG, int> Node;

ADG Astar(Simulator simulator);

int heuristic(Simulator simulator);