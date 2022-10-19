#include <queue>
#include <chrono>
using namespace std::chrono;

#include "../ADG/ADG_utilities.h"
#include "simulator.h"

typedef tuple<Simulator, int, int> Node;

ADG Astar(ADG root);

int heuristic(Simulator simulator);