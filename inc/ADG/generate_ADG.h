#pragma once
#include <iostream>
#include <fstream>
#include <string>

#include "../graph/graph.h"
#include "../types.h"
#include "ADG_utilities.h"

using namespace std;

bool same_locations(Location location1, Location location2);

ADG construct_ADG(const char* fileName);

ADG construct_ADG(shared_ptr<Paths> paths, bool duplication=false);

ADG construct_delayed_ADG(ADG &adg, int dlow, int dhigh, vector<int> &delayed_agents, vector<int> &states, int & input_sw_cnt, ofstream &outFile_setup);

ADG construct_delayed_ADG(ADG &adg, vector<int> & delay_steps, vector<int> &states, int & input_sw_cnt);
