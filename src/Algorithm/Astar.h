#include <queue>
#include <chrono>
using namespace std::chrono;
#include <chrono>
using namespace std::chrono;

#include "../ADG/ADG_utilities.h"
#include "simulator.h"

typedef tuple<Simulator, int, int> Node;

ADG Astar(Simulator simulator);

int heuristic(Simulator simulator);