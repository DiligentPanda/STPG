#pragma once
#include <string>
#include <vector>
#include "graph/graph.h"
#include "nlohmann/json.hpp"
#include "solver.h"
#include <chrono>

namespace ConstrainedGraph {

class ConstrainedGraphSolver: public Solver {
public:
    ConstrainedGraphSolver(float _time_limit);
    void solve_cg(
        const shared_ptr<Graph> & graph,
        const std::string & map_fp,
        const std::string & scen_fp,
        const std::string & path_fp,
        const std::vector<int> & states,
        const std::vector<int> & delay_steps
    );

    shared_ptr<Graph> solve(const shared_ptr<Graph> & graph){return nullptr;};
    void write_stats(nlohmann::json & stats) {};
    shared_ptr<GroupManager> & get_group_manager() {return group_manager;};

    shared_ptr<GroupManager> group_manager;
    float time_limit;
    std::chrono::microseconds searchT = std::chrono::microseconds::zero();
};

};