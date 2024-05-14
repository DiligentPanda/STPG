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

    bool use_grouping = true;
    GroupingMethod grouping_method = GroupingMethod::SIMPLE; // this is the default method used by the milp paper
    float time_limit=90.0;
    float eps=0.0;
    shared_ptr<py::scoped_interpreter> guard;
    shared_ptr<py::object> solver;
    shared_ptr<GroupManager> group_manager;

    // stats
    int vertex_cnt = 0;
    int sw_edge_cnt = 0;
    microseconds groupingT = std::chrono::microseconds::zero();
    microseconds searchT = std::chrono::microseconds::zero();

    MILPSolver(const string & _grouping_method, float _time_limit, float _eps=0.0):
        time_limit(_time_limit), 
        eps(_eps) {

        if (_grouping_method=="none") {
            std::cout<<"no grouping is not supported in milp now"<<std::endl;
            exit(-1);
            use_grouping=false;
            grouping_method=GroupingMethod::NONE;
        } else if (_grouping_method=="simple") {
            use_grouping=true;
            grouping_method=GroupingMethod::SIMPLE;
        } else if (_grouping_method=="simple_merge") {
            use_grouping=true;
            grouping_method=GroupingMethod::SIMPLE_MERGE;
        } else if (_grouping_method=="all") {
            use_grouping=true;
            grouping_method=GroupingMethod::ALL;
        } else {
            std::cout<<"unknown grouping method: "<<_grouping_method<<std::endl;
            exit(19);
        }

        // python interpreter
        guard=make_shared<py::scoped_interpreter>();

        py::module_ milp_solver=py::module_::import("MILP.solver");
        solver=make_shared<py::object>(milp_solver.attr("MILPSolver")(time_limit, eps));
        solver->attr("test")();
    }

    shared_ptr<Graph> solve(const shared_ptr<Graph> & init_graph, COST_TYPE init_cost, vector<int> & curr_states) {
        searchT=microseconds((int64_t)(time_limit*1000000));
        vertex_cnt = init_graph->get_num_states();
        sw_edge_cnt = init_graph->get_num_switchable_edges();

        auto graph = init_graph->copy();

        // TODO(rivers): we should make grouping outside before the execution of an graph
        // TODO(rivers): make grouping method configurable
        group_manager=make_shared<GroupManager>(graph, curr_states, grouping_method);
        std::cout<<group_manager->groups.size()<<std::endl;

        if (use_grouping) {
            // std::cout<<"input_sw_cnt: "<<input_sw_cnt<<std::endl;
            auto start = high_resolution_clock::now();
            group_manager=std::make_shared<GroupManager>(graph, curr_states, grouping_method);
            auto end = high_resolution_clock::now();
            groupingT += duration_cast<microseconds>(end - start);
            std::cout<<"group size: "<<group_manager->groups.size()<<std::endl;
            // group_manager->print_groups();
        }

        // we need to pack graph into the format that python solver can understand
        int num_agents=graph->get_num_agents();

        // paths
        // TODO(rivers): we can start from states
        vector<vector<int> > paths;
        for (int agent_id=0; agent_id<num_agents; ++agent_id) {
            vector<int> path;
            for (int state_id=curr_states[agent_id];state_id<graph->get_num_states(agent_id);++state_id) {
                path.push_back(state_id);
            }
            paths.push_back(path);
        }

        // non_switchable_edges
        vector<tuple<int,int,int,int> > non_switchable_edges;
        for (int agent_id=0; agent_id<num_agents; ++agent_id) {
            for (int state_id=0; state_id<graph->get_num_states(agent_id); ++state_id) {
                for (auto & p: graph->get_non_switchable_in_neighbor_pairs(agent_id, state_id)) {
                    non_switchable_edges.emplace_back(p.first, p.second, agent_id, state_id);
                }
            }
        }

        // switchable_edge_groups
        std::unordered_map<int, vector<tuple<int,int,int,int> > > switchable_edge_groups;
        std::unordered_set<int> group_ids;

        for (int agent_id=0; agent_id<num_agents; ++agent_id) {
            for (int state_id=0; state_id<graph->get_num_states(agent_id); ++state_id) {
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

        for (int group_id: group_ids) {
            auto & group = group_manager->groups[group_id];
            vector<tuple<int,int,int,int> > group_edges;
            for (long e: group) {
                int out_idx=group_manager->get_out_idx(e);
                int in_idx=group_manager->get_in_idx(e);
                auto && out_pair=graph->get_agent_state_id(out_idx);
                auto && in_pair=graph->get_agent_state_id(in_idx);
                group_edges.emplace_back(out_pair.first, out_pair.second, in_pair.first, in_pair.second);
            }
            switchable_edge_groups[group_id]=group_edges;
        }

        py::object ret=solver->attr("solve")(paths, non_switchable_edges, switchable_edge_groups, init_cost);

        auto && ret_tuple=ret.cast<py::tuple>();
        int status=ret_tuple[0].cast<int>();
        COST_TYPE elapse=ret_tuple[1].cast<COST_TYPE>();
        COST_TYPE objective_value=ret_tuple[2].cast<COST_TYPE>();
        vector<int> && group_ids_to_reverse=ret_tuple[3].cast<vector<int> >();

        searchT=microseconds((int64_t)(elapse*1000000));
        if (status==0) {
            // success
            std::cout<<"Succeed. results: "<<status<<","<<elapse<<","<<objective_value<<std::endl;
            // reverse the groups
            for (int group_id: group_ids_to_reverse) {
                auto && edges = group_manager->get_groupable_edges(group_id);
                graph->fix_switchable_type2_edges(edges, true, false);
            }
        } else {
            std::cout<<"Fail to find solution within "<<time_limit<<std::endl;
        }

        graph->fix_all_switchable_type2_edges();
        
        return graph;
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