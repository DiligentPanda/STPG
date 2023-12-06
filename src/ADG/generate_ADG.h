#include <iostream>
#include <fstream>
#include <string>

#include "../graph/graph.h"
#include "../types.h"
#include "ADG_utilities.h"

using namespace std;

bool same_locations(Location location1, Location location2);

ADG construct_ADG(char* fileName);

ADG construct_delayed_ADG(ADG &adg, int delay, int delayed_agent, int delayed_state);
