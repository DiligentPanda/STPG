#pragma once
#include <iostream>
#include <fstream>
#include <string>

#include "graph/graph.h"

using namespace std;

shared_ptr<Graph> construct_graph(const char* fileName);

shared_ptr<Graph> construct_graph(const shared_ptr<Paths> & paths, bool duplication=false);

shared_ptr<Graph> construct_delayed_graph(const shared_ptr<Graph> & graph, const vector<int> & delay_steps, const vector<int> &states);
