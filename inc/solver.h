#pragma once
#include "nlohmann/json.hpp"
#include "graph/graph.h"
#include "group/group.h"

// abstract class for switchable TPG optimization
class Solver {
public:
    virtual shared_ptr<Graph> solve(const shared_ptr<Graph> & graph) = 0;
    virtual void write_stats(nlohmann::json & stats)=0;
    virtual shared_ptr<GroupManager> & get_group_manager()=0;
};