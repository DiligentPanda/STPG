#include <queue>
#include <chrono>
using namespace std::chrono;

#include "../ADG/ADG_utilities.h"
#include "simulator.h"

typedef tuple<ADG, int> Node;

ADG Astar(ADG adg);

int heuristic(ADG adg, vector<int> *ts);