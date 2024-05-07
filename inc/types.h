#pragma once

#include <stdlib.h>
#include <tuple>
#include <vector>
#include <set>
#include <utility>
#include <memory>
#include <iostream>
#include <map>

// TODO(rivers): bad habit. fix this.
using namespace std;

// <coordinates>
typedef pair<int, int> Location;

// <location, timestep>
typedef vector<pair<Location, int>> Path;

// paths for all agents
typedef vector<Path> Paths;

// <arr[time]=vertex, arr[vertex]=time>, used for topological sort
typedef pair<
    shared_ptr<vector<int> >, 
    shared_ptr<vector<int> > 
> sortResult;

// only keep edges
struct Subgraph {
    // maybe list is enough, such that we don't need set
    // the first vector is a vector of agents
    // the second vector is a vector of states of this agent
    // the set saves another side of the edge
    // TODO(rivers): maybe we could replace the second vector with a unordered_map, because the edges might be sparse on nodes.
    int total_edge_cnt;
    shared_ptr<vector<int> > accum_state_cnts_begin;
    shared_ptr<vector<pair<int,int> > > global_state_id_to_agent_state_ids;
    vector<shared_ptr<vector<set<int> > > > out_neighbors;
    vector<shared_ptr<vector<set<int> > > > in_neighbors; 

    Subgraph(int num_agents, const shared_ptr<vector<int> > & accum_state_cnts_begin, const shared_ptr<vector<int> > & state_cnts, const shared_ptr<vector<pair<int,int> > > & gid_to_aid_lids): 
        total_edge_cnt(0), 
        accum_state_cnts_begin(accum_state_cnts_begin),
        global_state_id_to_agent_state_ids(gid_to_aid_lids) {
        out_neighbors.resize(num_agents);
        in_neighbors.resize(num_agents);
        for (auto i=0;i<num_agents;i++) {
            out_neighbors[i] = make_shared<vector<set<int> > >((*state_cnts)[i]);
            in_neighbors[i] = make_shared<vector<set<int> > >((*state_cnts)[i]);
        }
    }

    bool has_edge(int global_state_from, int global_state_to) {
        int agent_from, state_from;
        std::tie(agent_from, state_from)=get_agent_state_id(global_state_from);
        bool found=out_neighbors[agent_from]->at(state_from).count(global_state_to)>0;
        return found;
    }

    bool has_edge(int agent_from, int state_from, int agent_to, int state_to) {
        int state_to_global_id=get_global_state_id(agent_to, state_to);
        bool found=out_neighbors[agent_from]->at(state_from).count(state_to_global_id)>0;
        return found;
    }

    bool insert_edge(int agent_from, int state_from, int agent_to, int state_to, bool check=true) {
        int state_from_global_id=get_global_state_id(agent_from, state_from);
        int state_to_global_id=get_global_state_id(agent_to, state_to);
        auto pair=(*out_neighbors[agent_from])[state_from].insert(state_to_global_id);
        if (check && !pair.second) {
            cout << "Error: edge already exists" << endl;
            exit(1);
        }
        pair=(*in_neighbors[agent_to])[state_to].insert(state_from_global_id);
        if (check && !pair.second) {
            cout << "Error: edge already exists" << endl;
            exit(1);
        }
        if (pair.second) {
            ++total_edge_cnt;
        }
        return pair.second;
    }

    bool remove_edge(int agent_from, int state_from, int agent_to, int state_to, bool check=true) {
        int state_from_global_id=get_global_state_id(agent_from, state_from);
        int state_to_global_id=get_global_state_id(agent_to, state_to);
        auto num=(*out_neighbors[agent_from])[state_from].erase(state_to_global_id);
        if (check && num==0) {
            cout << "Error: edge from ("<<agent_from<<","<<state_from<<") to ("<<agent_to<<","<<state_to<<") not found in out_neighbors" << endl;
            exit(1);
        }

        num=(*in_neighbors[agent_to])[state_to].erase(state_from_global_id);
        if (check && num==0) {
            cout << "Error: edge from ("<<agent_from<<","<<state_from<<") to ("<<agent_to<<","<<state_to<<") not found in in_neighbors" << endl;
            exit(1);
        }
        total_edge_cnt-=num;
        
        return num>0;
    }

    inline int get_global_state_id(int agent_id, int local_state_id) {
        return (*accum_state_cnts_begin)[agent_id]+local_state_id;
    }

    inline pair<int,int> get_agent_state_id(int global_state_id) {
        return (*global_state_id_to_agent_state_ids)[global_state_id];
    }   

    inline int get_agent_id(int global_state_id) {
        return (*global_state_id_to_agent_state_ids)[global_state_id].first;
    }   
    
    inline int get_state_id(int global_state_id) {
        return (*global_state_id_to_agent_state_ids)[global_state_id].second;
    }   

    inline set<int> & get_in_neighbor_global_ids(int agent, int state) {
        return (*in_neighbors[agent])[state];
    }

    inline set<int> & get_out_neighbor_global_ids(int agent, int state) {
        return (*out_neighbors[agent])[state];
    }

    inline set<int> & get_in_neighbor_global_ids(int global_state_id) {
        auto && p=get_agent_state_id(global_state_id);
        return (*in_neighbors[p.first])[p.second];
    }

    inline set<int> & get_out_neighbor_global_ids(int global_state_id) {
        auto && p=get_agent_state_id(global_state_id);
        return (*out_neighbors[p.first])[p.second];
    }

    bool sanity_check() {
        for (int agent=0;agent<(int)out_neighbors.size();++agent) {
            for (int state=0;state<(int)out_neighbors[agent]->size();++state) {
                int global_state_id=get_global_state_id(agent, state);
                for (auto neighbor: (*out_neighbors[agent])[state]) {
                    auto && p=get_agent_state_id(neighbor);
                    if ((*in_neighbors[p.first])[p.second].count(global_state_id)==0) {
                        //cout<<"Error: edge not found in in_neighbors"<<endl;
                        return false;
                    }
                }
            }
        }
        for (int agent=0;agent<(int)in_neighbors.size();++agent) {
            for (int state=0;state<(int)in_neighbors[agent]->size();++state) {
                int global_state_id=get_global_state_id(agent, state);
                for (auto neighbor: (*in_neighbors[agent])[state]) {
                    auto && p=get_agent_state_id(neighbor);
                    if ((*out_neighbors[p.first])[p.second].count(global_state_id)==0) {
                        //cout<<"Error: edge not found in out_neighbors"<<endl;
                        return false;
                    }
                }
            }
        }
        return true;
    }

};


struct Graph {
    shared_ptr<Paths> paths;
    shared_ptr<vector<pair<int,int> > > global_state_id_to_agent_state_ids;

    shared_ptr<Subgraph> type1_edges;
    shared_ptr<Subgraph> non_switchable_type2_edges;
    shared_ptr<Subgraph> switchable_type2_edges;

    int total_state_cnt;
    shared_ptr<vector<int> > state_cnts;
    shared_ptr<vector<int> > accum_state_cnts_begin;
    shared_ptr<vector<int> > accum_state_cnts_end;
 

    Graph(const shared_ptr<Paths> & _paths): 
        paths(_paths) {
        const vector<int> curr_states(get_num_agents(),0);
        _init(curr_states);
    }

    Graph(const shared_ptr<Paths> & _paths, const vector<int> & curr_states): 
        paths(_paths) {
        _init(curr_states);
    }

    void _copy_type2_edges(const shared_ptr<Graph> & new_graph, int agent) {
        new_graph->non_switchable_type2_edges->out_neighbors[agent]=make_shared<vector<set<int> > >(
            *(non_switchable_type2_edges->out_neighbors[agent])
        );

        new_graph->non_switchable_type2_edges->in_neighbors[agent]=make_shared<vector<set<int> > >(
            *(non_switchable_type2_edges->in_neighbors[agent])
        );

        new_graph->switchable_type2_edges->out_neighbors[agent]=make_shared<vector<set<int> > >(
            *(switchable_type2_edges->out_neighbors[agent])
        );

        new_graph->switchable_type2_edges->in_neighbors[agent]=make_shared<vector<set<int> > >(
            *(switchable_type2_edges->in_neighbors[agent])
        );
    }

    shared_ptr<Graph> copy() {
        // copy the graph: used in the search
        auto new_graph=make_shared<Graph>(*this);
       
        // the type2 edges need to be copied carefully
        new_graph->non_switchable_type2_edges=make_shared<Subgraph>(*non_switchable_type2_edges);
        new_graph->switchable_type2_edges=make_shared<Subgraph>(*switchable_type2_edges);
       
       for (int agent=0; agent<get_num_agents();++agent) {
            _copy_type2_edges(new_graph, agent);
       }

        return new_graph;
    }

    shared_ptr<Graph> copy(int agent1, int agent2) {
        
        // copy the graph: used in the search
        auto new_graph=make_shared<Graph>(*this);
       
        // the type2 edges need to be copied carefully
        new_graph->non_switchable_type2_edges=make_shared<Subgraph>(*non_switchable_type2_edges);
        new_graph->switchable_type2_edges=make_shared<Subgraph>(*switchable_type2_edges);
       
        _copy_type2_edges(new_graph, agent1);
        _copy_type2_edges(new_graph, agent2);

        return new_graph;
    }

    inline int get_num_states(int agent_id=-1) {
        if (agent_id<0)
            return total_state_cnt;
        else
            return (*state_cnts)[agent_id];
    }

    inline int get_num_agents() {
        return paths->size();
    }

    inline int get_num_switchable_edges() {
        return switchable_type2_edges->total_edge_cnt;
    }

    inline int get_global_state_id(int agent_id, int local_state_id) {
        return (*accum_state_cnts_begin)[agent_id]+local_state_id;
    }

    inline pair<int,int> get_agent_state_id(int global_state_id) {
        return (*global_state_id_to_agent_state_ids)[global_state_id];
    }   

    inline int get_agent_id(int global_state_id) {
        return (*global_state_id_to_agent_state_ids)[global_state_id].first;
    }   
    
    inline int get_state_id(int global_state_id) {
        return (*global_state_id_to_agent_state_ids)[global_state_id].second;
    }   
    
    void _init(const vector<int> & curr_states) {
       
        total_state_cnt = 0;
        state_cnts = make_shared<vector<int> >();
        accum_state_cnts_begin = make_shared<vector<int> >();
        accum_state_cnts_end = make_shared<vector<int> >();
        for (auto & path: *paths) {
            state_cnts->push_back(path.size());
            accum_state_cnts_begin->push_back(total_state_cnt);
            total_state_cnt += path.size();
            accum_state_cnts_end->push_back(total_state_cnt);
        }

        int num_agents=get_num_agents();

        global_state_id_to_agent_state_ids=make_shared<vector<pair<int,int>>>();
        for (int agent_id=0;agent_id<num_agents;++agent_id) {
            for (int state_id=0;state_id<(*state_cnts)[agent_id];++state_id) {
                global_state_id_to_agent_state_ids->emplace_back(agent_id,state_id);
            }
        }

        type1_edges = make_shared<Subgraph>(num_agents, accum_state_cnts_begin, state_cnts, global_state_id_to_agent_state_ids);
        non_switchable_type2_edges = make_shared<Subgraph>(num_agents, accum_state_cnts_begin, state_cnts, global_state_id_to_agent_state_ids);
        switchable_type2_edges = make_shared<Subgraph>(num_agents, accum_state_cnts_begin, state_cnts, global_state_id_to_agent_state_ids);
        
        _build_type1_edges();
        _build_type2_edges();

        fix_edges_after_last_states();
        fix_edges_before_curr_states(curr_states);

    }

    void _build_type1_edges() {
        for (auto agent=0;agent<get_num_agents();++agent) {
            int state_cnt=(*state_cnts)[agent];
            for (auto state=1;state<state_cnt;++state) {
                type1_edges->insert_edge(agent, state-1, agent, state);
            }
        }
    }

    void _build_type2_edges() {
        for (auto agent1=0;agent1<get_num_agents();++agent1) {
            Path & path1=(*paths)[agent1];
            int num_states1=path1.size();
            for (auto agent2=agent1+1;agent2<get_num_agents();++agent2) {
                Path & path2=(*paths)[agent2];
                int num_states2=path2.size();
                for (auto state1=0;state1<num_states1;++state1) {
                    for (auto state2=0;state2<num_states2;++state2) {
                        auto & pair1 = path1[state1];
                        auto & pair2 = path2[state2];

                        // if the same location and pair1 & pair 2 are not delayed nodes.
                        if (pair1.first==pair2.first && pair1.second>=0 and pair2.second>=0) {
                            // if the same timestep
                            if (pair1.second==pair2.second) {
                                std::cout<<"Error: two agents are at the same location at the same time"<<std::endl;
                                exit(-1);
                            } else if (pair1.second<pair2.second) {
                                // TODO(rivers): this is a bug, we need to add dummy ending nodes to encode the dependency from the last state
                                if (state1<num_states1-1) {
                                    // type 2 edge always points to the later state
                                    switchable_type2_edges->insert_edge(agent1, state1+1, agent2, state2);
                                }
                            } else {
                                if (state2<num_states2-1) {
                                    switchable_type2_edges->insert_edge(agent2, state2+1, agent1, state1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    pair<int,int> fix_switchable_type2_edge(int state_from, int state_to, bool reverse=false, bool check=true) {
        int agent1, state1, agent2, state2;
        std::tie(agent1, state1)=get_agent_state_id(state_from);
        std::tie(agent2, state2)=get_agent_state_id(state_to);
        bool succ=_fix_switchable_type2_edge(agent1, state1, agent2, state2, reverse, check);

        if (succ) {
            if (reverse) {
                return {state_to+1,state_from-1};
            } else {
                return {state_from, state_to};
            }
        } else {
            return {-1,-1};
        }
    }

    vector<pair<int,int> > fix_switchable_type2_edges(const vector<pair<int,int> > & edges, bool reverse=false, bool check=true) {
        vector<pair<int,int> > fixed_edges;
        for (auto & edge: edges) {
            auto && p=fix_switchable_type2_edge(edge.first, edge.second, reverse, check);
            // Otherwise, the edge is probably fixed earlier
            if (p.first>=0)
                fixed_edges.emplace_back(p);
        }
        return fixed_edges;
    }

    bool _fix_switchable_type2_edge(int agent_from, int state_from, int agent_to, int state_to, bool reverse=false, bool check=true) {
        bool succ=switchable_type2_edges->remove_edge(agent_from, state_from, agent_to, state_to, check);
        if (check && !succ) {
            cout << "Error: edge not found" << endl;
            exit(1);
        }
        if (succ) {
            if (reverse) {
                non_switchable_type2_edges->insert_edge(agent_to, state_to+1, agent_from, state_from-1, check);
            } else {
                non_switchable_type2_edges->insert_edge(agent_from, state_from, agent_to, state_to, check);
            }
        }
        return succ;
    }

    void fix_edges_after_last_states() {
        // need to fix edges that are not switchable because of we cannot point to a state after the end state.
        auto & subgraph=*switchable_type2_edges;
        for (auto agent_id=0;agent_id<get_num_agents();++agent_id) {
            auto last_state_id=(*state_cnts)[agent_id]-1;
            // find those who point to the last state
            auto state_from_global_ids=(*subgraph.in_neighbors[agent_id])[last_state_id];
            for (auto state_from_global_id: state_from_global_ids) {
                auto && p=subgraph.get_agent_state_id(state_from_global_id);
                _fix_switchable_type2_edge(p.first, p.second, agent_id, last_state_id, false);
            }
        }
    }

    void fix_edges_before_curr_states(const vector<int> & curr_states) {
        // need to fix edges that are point from before the next state of a current state
        // NOTE(rivers): we don't need care about edges pointing to a current state
        // because according to the definition of type 2 edges, the begin node's state must also be completed.
        // otherwise the plan itself is invalid.
        auto & subgraph=*switchable_type2_edges;
        for (auto agent_id=0;agent_id<get_num_agents();++agent_id) {
            auto curr_state_id=curr_states[agent_id];
            auto next_state_id=curr_state_id;
            if (curr_state_id<(*state_cnts)[agent_id]-1) {
                ++next_state_id;
            }
            for (auto state_id=0;state_id<=next_state_id;++state_id) {
                // find those who are pointed by the state before the next state of a current state
                auto state_to_global_ids=(*subgraph.out_neighbors[agent_id])[state_id];
                for (auto state_to_global_id: state_to_global_ids) {
                    auto && p=subgraph.get_agent_state_id(state_to_global_id);
                    // we set to false because there might be some overlaps with other calls to fix edges
                    _fix_switchable_type2_edge(agent_id, state_id, p.first, p.second, false);
                }
            }
        }
    }


    set<int> get_out_neighbor_global_ids(int global_state_id) {

        set<int> out_neighbor_global_ids = non_switchable_type2_edges->get_out_neighbor_global_ids(global_state_id);
        set<int> & type1_edge_global_ids = type1_edges->get_out_neighbor_global_ids(global_state_id);
        out_neighbor_global_ids.insert(type1_edge_global_ids.begin(), type1_edge_global_ids.end());

        return out_neighbor_global_ids;
    }

    set<int> get_in_neighbor_global_ids(int global_state_id) {

        set<int> in_neighbor_global_ids = non_switchable_type2_edges->get_in_neighbor_global_ids(global_state_id);
        set<int> & type1_edge_global_ids = type1_edges->get_in_neighbor_global_ids(global_state_id);
        in_neighbor_global_ids.insert(type1_edge_global_ids.begin(), type1_edge_global_ids.end());

        return in_neighbor_global_ids;
    }

    size_t get_in_degree(int global_state_id) {

        set<int> & in_neighbor_global_ids = non_switchable_type2_edges->get_in_neighbor_global_ids(global_state_id);
        set<int> & type1_edge_global_ids = type1_edges->get_in_neighbor_global_ids(global_state_id);
        
        return in_neighbor_global_ids.size() + type1_edge_global_ids.size();
    }

    vector<pair<int,int> > get_non_switchable_in_neighbor_pairs(int agent, int state) {
        auto & type2_in_neighbor_global_ids = non_switchable_type2_edges->get_in_neighbor_global_ids(agent, state);
        auto & type1_in_neighbor_global_ids = type1_edges->get_in_neighbor_global_ids(agent, state);

        vector<pair<int,int> > in_neighbor_pairs;
        for (auto in_neighbor_global_id: type2_in_neighbor_global_ids) {
            in_neighbor_pairs.emplace_back(
                non_switchable_type2_edges->get_agent_state_id(in_neighbor_global_id)
            );
        }
        for (auto in_neighbor_global_id: type1_in_neighbor_global_ids) {
            in_neighbor_pairs.emplace_back(
                type1_edges->get_agent_state_id(in_neighbor_global_id)
            );
        }
        return in_neighbor_pairs;
    }

    // returns a vector of <agent_id, local_state_id>, which are pointint to this <agent, state>
    vector<pair<int,int> > get_non_switchable_type2_in_neighbor_pairs(int agent, int state) {
        auto & in_neighbor_global_ids = non_switchable_type2_edges->get_in_neighbor_global_ids(agent, state);

        vector<pair<int,int> > in_neighbor_pairs;
        for (auto in_neighbor_global_id: in_neighbor_global_ids) {
            in_neighbor_pairs.emplace_back(
                non_switchable_type2_edges->get_agent_state_id(in_neighbor_global_id)
            );
        }
        return in_neighbor_pairs;
    }

    vector<pair<int,int> > get_type2_in_neighbor_pairs(int agent, int state) {
        auto & non_switchable_in_neighbor_global_ids = non_switchable_type2_edges->get_in_neighbor_global_ids(agent, state);
        auto & switchable_in_neighbor_global_ids = switchable_type2_edges->get_in_neighbor_global_ids(agent, state);

        vector<pair<int,int> > in_neighbor_pairs;
        for (auto in_neighbor_global_id: non_switchable_in_neighbor_global_ids) {
            in_neighbor_pairs.emplace_back(
                non_switchable_type2_edges->get_agent_state_id(in_neighbor_global_id)
            );
        }

        for (auto in_neighbor_global_id: switchable_in_neighbor_global_ids) {
            in_neighbor_pairs.emplace_back(
                switchable_type2_edges->get_agent_state_id(in_neighbor_global_id)
            );
        }

        return in_neighbor_pairs;
    }

    void fix_all_switchable_type2_edges() {
        std::vector<pair<int,int> > edges_to_fix;
        for (int i = 0; i < get_num_states(); ++i) {
            auto & outNeib = switchable_type2_edges->get_out_neighbor_global_ids(i);
            for (auto j : outNeib) {
                edges_to_fix.emplace_back(i,j);
            }
        }
        for (auto & edge: edges_to_fix) {
            fix_switchable_type2_edge(edge.first, edge.second, false, true);
        }
    }

};
