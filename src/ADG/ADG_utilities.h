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

// Nothing happens if the type2 edge does not exist in the first place
void switch_type2_edge(ADG adg, int agent1, int state1, int agent2, int state2);

// Return an vector of agent-state pairs
vector<pair<int, int>> get_type2_inNeighbors(ADG adg, int agent, int state);

vector<pair<int, int>> get_type2_outNeighbors(ADG adg, int agent, int state);

Location get_state_target(ADG adg, int agent, int state);