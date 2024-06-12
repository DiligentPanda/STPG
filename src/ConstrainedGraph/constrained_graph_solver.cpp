#include "ConstrainedGraph/constrained_graph_solver.h"
#include "ConstrainedGraph/DelayInstance.h"
#include "ConstrainedGraph/CBS/CBS.h"
#include "graph/graph.h"

namespace ConstrainedGraph {

ConstrainedGraphSolver::ConstrainedGraphSolver(float _time_limit) : time_limit(_time_limit) {

}

void ConstrainedGraphSolver::solve_cg(
    const shared_ptr<Graph> & graph,
    const std::string & map_fp,
    const std::string & scen_fp,
    const std::string & path_fp,
    const std::vector<int> & states,
    const std::vector<int> & delay_steps
) {
    std::cout<<"map_fp: "<<map_fp<<std::endl;
    std::cout<<"scen_fp: "<<scen_fp<<std::endl;
    std::cout<<"path_fp: "<<path_fp<<std::endl;

    int agent_num = (int)(states.size());

    // create delay instance
    ConstrainedGraph::DelayInstance* delay_instance = new DelayInstance(
        graph,
        map_fp, 
    	scen_fp, 
        path_fp,
        states,
        delay_steps
    );

    float init_cost=0;
    for (int i=0; i<agent_num; i++) {
        init_cost += ((float) delay_instance->postDelayPlan[i].size()-1);
    }

    // create planner
    // TODO(rivers): what is this used for?
    delay_instance->numDelays = 0;
    bool improvements = true;
    CBS cbs(delay_instance, false, 0);
    cbs.setPrioritizeConflicts(improvements);
    cbs.setDisjointSplitting(false);
    cbs.setBypass(improvements);
    cbs.setRectangleReasoning(improvements);
    // TODO(rivers): why not use corridor reasoning?
    cbs.setCorridorReasoning(false);
    cbs.setHeuristicType(improvements? heuristics_type::WDG : heuristics_type::ZERO, heuristics_type::ZERO);
    cbs.setTargetReasoning(improvements);
    cbs.setMutexReasoning(false);
    cbs.setConflictSelectionRule(conflict_selection::EARLIEST);
    cbs.setNodeSelectionRule(node_selection::NODE_CONFLICTPAIRS); //
    cbs.setSavingStats(false);
    cbs.setHighLevelSolver(high_level_solver_type::ASTAREPS, 1.0);

    cbs.solve(time_limit, 0, MAX_COST);

    // TODO
    double search_time = cbs.runtime;

    float final_cost=0;
    if (cbs.solution_found && cbs.validateSolution()) {
        // TODO: save the cost
        final_cost=cbs.solution_cost;
        std::cout << "Solution found" << std::endl;
    } else {
        // TODO
        final_cost=init_cost;
        std::cout << "Solution not found" << std::endl;
    }

    std::cout<<"init cost: "<<init_cost<<std::endl;
    std::cout<<"final cost: "<<final_cost<<std::endl;

    // build a graph and return for TPG execution

}

};
