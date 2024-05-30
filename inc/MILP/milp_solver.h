#pragma once
#include "graph/graph.h"
#include <pybind11/embed.h> // everything needed for embedding
#include <pybind11/stl.h> // for conversion between C++ and Python types
#include <unordered_map>
#include <unordered_set>
#include "group/group.h"
#include "solver.h"
#include <chrono>
using namespace std::chrono;

namespace py = pybind11;

class MILPSolver: public Solver {
public:

    float time_limit=90.0;
    bool use_grouping = true;
    int horizon=-1;
    shared_ptr<GroupManager> group_manager;
    float eps=0.0;
    shared_ptr<py::scoped_interpreter> guard;
    shared_ptr<py::object> solver;


    // stats
    int vertex_cnt = 0;
    int sw_edge_cnt = 0;
    microseconds groupingT = std::chrono::microseconds::zero();
    microseconds searchT = std::chrono::microseconds::zero();

    MILPSolver(float _time_limit, int _horizon=-1, std::shared_ptr<GroupManager> _group_manager=nullptr, float _eps=0.0):
        time_limit(_time_limit), 
        use_grouping(_group_manager.get()!=nullptr),
        horizon(_horizon),
        group_manager(_group_manager),
        eps(_eps) {

        if (!use_grouping) {
            std::cerr<<"MILPSolver must use grouping"<<std::endl;
            exit(-1);
        }

        // python interpreter
        guard=make_shared<py::scoped_interpreter>();

        py::module_ milp_solver=py::module_::import("py_milp.solver");
        solver=make_shared<py::object>(milp_solver.attr("MILPSolver")(time_limit, eps));
        solver->attr("test")();
    }

    shared_ptr<GroupManager> & get_group_manager() {
      return this->group_manager;
    }

    shared_ptr<Graph> solve(const shared_ptr<Graph> & _graph) {
        if (!_graph->is_fixed()) {
            std::cout<<"Astar:: graph is not fixed"<<std::endl;
            exit(100);
        }

        auto init_graph = _graph;
        // since many operations are in-place, should never operate on the original graph. make a copy first.
        auto graph = _graph->copy();

        if (horizon!=-1) {
            graph->make_switchable(horizon, group_manager);
        } else {
            graph->make_switchable();
        }
        
        searchT=microseconds((int64_t)(time_limit*1000000));
        vertex_cnt = graph->get_num_states();
        sw_edge_cnt = graph->get_num_switchable_edges();

        // we need to pack graph into the format that python solver can understand
        int num_agents=graph->get_num_agents();

        // paths
        // TODO(rivers): we can start from states
        vector<vector<int> > paths;
        for (int agent_id=0; agent_id<num_agents; ++agent_id) {
            vector<int> path;
            int num_states=graph->get_num_states(agent_id);
            for (int state_id=graph->curr_states->at(agent_id);state_id<num_states;++state_id) {
                path.push_back(state_id);
            }
            paths.push_back(path);
        }

        // non_switchable_edges
        vector<tuple<int,int,int,int,COST_TYPE> > non_switchable_edges;
        for (int agent_id=0; agent_id<num_agents; ++agent_id) {
            for (int state_id=graph->curr_states->at(agent_id); state_id<graph->get_num_states(agent_id); ++state_id) {
                for (auto & p: graph->get_non_switchable_in_neighbor_pairs(agent_id, state_id)) {
                    int in_global_state_id=graph->get_global_state_id(p.first, p.second);
                    int global_state_id=graph->get_global_state_id(agent_id, state_id);
                    COST_TYPE cost=graph->edge_manager->get_edge(in_global_state_id, global_state_id).cost;
                    non_switchable_edges.emplace_back(p.first, p.second, agent_id, state_id, cost);
                }
            }
        }

        // switchable_edge_groups
        std::unordered_map<int, vector<tuple<int,int,int,int,COST_TYPE> > > switchable_edge_groups;
        std::unordered_set<int> group_ids;

        for (int agent_id=0; agent_id<num_agents; ++agent_id) {
            for (int state_id=graph->curr_states->at(agent_id); state_id<graph->get_num_states(agent_id); ++state_id) {
                int global_state_id=graph->switchable_type2_edges->get_global_state_id(agent_id, state_id);
                for (int in_neighbor: graph->switchable_type2_edges->get_in_neighbor_global_ids(global_state_id)) {
                    int group_id = group_manager->get_group_id(in_neighbor, global_state_id);
                    if (group_id==-1) {
                        cout<<"error: group_id==-1. group not found for edge: "<<in_neighbor<<"->"<<global_state_id<<endl;
                        exit(-1);
                    }
                    group_ids.insert(group_id);
                }
            }
        }

        std::cout << "vertex_cnt = " << vertex_cnt << ", sw_edge_cnt = " << sw_edge_cnt<<", sw_edge_groups_cnt = "<< group_ids.size() << "\n";

        for (int group_id: group_ids) {
            auto & group = group_manager->groups[group_id];
            vector<tuple<int,int,int,int,COST_TYPE> > group_edges;
            for (long e: group) {
                int out_idx=group_manager->get_out_idx(e);
                int in_idx=group_manager->get_in_idx(e);
                if (!(graph->switchable_type2_edges->has_edge(out_idx, in_idx))) {
                    continue;
                }
                COST_TYPE cost=graph->edge_manager->get_edge(out_idx, in_idx).cost;
                auto && out_pair=graph->get_agent_state_id(out_idx);
                auto && in_pair=graph->get_agent_state_id(in_idx);
                group_edges.emplace_back(out_pair.first, out_pair.second, in_pair.first, in_pair.second, cost);
            }
            switchable_edge_groups[group_id]=group_edges;
        }

        py::object ret=solver->attr("solve")(paths, non_switchable_edges, switchable_edge_groups);

        auto && ret_tuple=ret.cast<py::tuple>();
        int status=ret_tuple[0].cast<int>();
        COST_TYPE elapse=ret_tuple[1].cast<COST_TYPE>();
        COST_TYPE objective_value=ret_tuple[2].cast<COST_TYPE>();
        vector<int> && group_ids_to_reverse=ret_tuple[3].cast<vector<int> >();

        searchT=microseconds((int64_t)(elapse*1000000));
        if (status==0) {
            // success
            std::cout<<"Succeed. results: "<<status<<", "<<elapse<<", "<<objective_value<<std::endl;
            // reverse the groups
            for (int group_id: group_ids_to_reverse) {
                auto && edges = group_manager->get_groupable_edges(group_id);
                graph->fix_switchable_type2_edges(edges, true, false);
            }
            graph->fix_all_switchable_type2_edges();
            return graph;

        } else {
            std::cout<<"Fail to find solution within "<<time_limit<<std::endl;
            return init_graph;
        }
    }

    void write_stats(nlohmann::json & stats) {
        stats["vertex"]=vertex_cnt;
        stats["sw_edge"]=sw_edge_cnt;
        stats["grouping_time"]=groupingT.count();
        stats["search_time"]=searchT.count();
        if (use_grouping) {
            auto & groups=group_manager->groups;
            stats["group"]=groups.size();
            stats["group_merge_edge"]=group_manager->group_merge_edge_cnt;
            COST_TYPE group_size_max=0;
            COST_TYPE group_size_avg=0;
            COST_TYPE group_size_min=sw_edge_cnt;
            for (int i=0;i<(int)groups.size();++i) {
            if((int)groups[i].size()>group_size_max) {
                group_size_max=groups[i].size();
            }
            if ((int)groups[i].size()<group_size_min) {
                group_size_min=groups[i].size();
            }
            group_size_avg+=groups[i].size();
            }
            group_size_avg/=groups.size();
            stats["group_size_max"]=group_size_max;
            stats["group_size_min"]=group_size_min;
            stats["group_size_avg"]=group_size_avg;
        } else {
            stats["group"]=sw_edge_cnt;
            stats["group_merge_edge"]=0;
            stats["group_size_max"]=1;
            stats["group_size_min"]=1;
            stats["group_size_avg"]=1;
        }
    }

};