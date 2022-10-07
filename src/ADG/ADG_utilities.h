#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "../graph/graph.h"
#include "../types.h"

using namespace std;

int get_agentCnt(ADG adg);

int get_stateCnt(ADG adg, int agent);

// Directed
bool is_type2_edge(ADG adg, int agent1, int state1, int agent2, int state2);

// Return NULL_LOCATION is there's no type2 edge between the two agent-state pairs
Location get_type2_edge_target