#pragma once
#include "nlohmann/json.hpp"
#include "graph/graph.h"

// abstract class for switchable TPG optimization
class Solver {
public:
    virtual shared_ptr<Graph> solve(const shared_ptr<Graph> & adg, COST_TYPE init_cost, vector<int> & states) = 0;
    virtual void write_stats(nlohmann::json & stats)=0;
};