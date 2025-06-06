#pragma once
#include "define.h"
#include <unordered_map>
#include <memory>
#include "graph/graph.h"
#include <random>
#include "solver.h"
#include "Algorithm/graph_algo.h"

class AgentState {
public:
    int state_id;
    int delay_steps;
    // agent_id, state_id. dependencies of the next state.
    std::unordered_map<int, int> dependencies;

    AgentState(): state_id(0), delay_steps(0) {}
};


enum DelayType {
    NO_DELAY,
    UNIFORM_TEMPORAL_DELAY,
    UNIFORM_STATE_DELAY,
    DURATION_SUBSET_DELAY
};



// NOTE: this is not a realistic simulator but only for the purpose of study in this project
// For simplicity, we only support integer time step here, but the planner is based one COST_TYPE, which can be float/double.
class NewSimulator {
public:

    std::shared_ptr<Solver> solver;

    float delay_prob;
    std::uniform_real_distribution<float> delay_distrib;
    std::uniform_int_distribution<int> delay_steps_distrib;
    int delay_steps_low;
    int delay_steps_high;
    int delay_duration;
    float delay_subset_portion; // x percent
    size_t seed;
    std::mt19937 rng;
    int verbose;

    DelayType delay_type=DelayType::UNIFORM_STATE_DELAY;

    std::vector<AgentState> agent_states;
    // graph is actually the control policy
    std::shared_ptr<Graph> graph;

    std::shared_ptr<std::vector<std::vector<int> > > spatial_delays;

    std::vector<bool> arrived;
    std::vector<std::vector<Location> > paths;

    int step_ctr;
    int total_delays;

    // we can use the same seed, but the execution would be dffereent anyway
    // we can only try more different seeds

    NewSimulator(
        const std::shared_ptr<Solver> & solver=nullptr,
        float delay_prob=-1,
        int delay_steps_low=0,
        int delay_steps_high=0,
        size_t _seed=0,
        int _verbose=0
        ): 
        solver(solver), 
        delay_prob(delay_prob),
        delay_distrib(0.0, 1.0),
        delay_steps_distrib(delay_steps_low, delay_steps_high),
        delay_steps_low(delay_steps_low),
        delay_steps_high(delay_steps_high),
        delay_duration(20),
        delay_subset_portion(0.5),
        seed(_seed), 
        rng(seed),
        verbose(_verbose) {
    }

    void set_seed(size_t _seed) {
        seed = _seed;
        rng.seed(_seed);
    }

    // once delayed we need to redo the search
    void update_graph(const std::vector<int> & _delays) {
        // modify the graph
        std::vector<int> curr_states;
        std::vector<COST_TYPE> delays;
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            curr_states.push_back(agent_states[agent_id].state_id);
            delays.push_back(_delays[agent_id]);
        }
        
        // std::cout<<"state updated"<<std::endl;

        graph->update_curr_states(curr_states);

        // std::cout<<"graph state updated"<<std::endl;

        graph->delay(delays);


        // auto switchable_graph = graph->copy();
        // switchable_graph->make_switchable(2, solver->get_group_manager());

        // auto new_simulator=std::make_shared<NewSimulator>();
        // auto fixed_switchable_graph=switchable_graph->get_fixed_version();
        // int cost =new_simulator->simulate(fixed_switchable_graph, curr_states);
        // std::cout<<"debug cost: "<<cost<<std::endl;

        // int state_from = graph->get_global_state_id(19, 2);
        // int state_to = graph->get_global_state_id(35, 1);
        // switchable_graph->fix_switchable_type2_edge(state_from,state_to,true,true);
        // auto fixed_switchable_graph2=switchable_graph->get_fixed_version();
        // int cost2 =new_simulator->simulate(fixed_switchable_graph2, curr_states);
        // std::cout<<"debug cost2: "<<cost2<<std::endl;

      

        // std::cout<<"graph udpated"<<std::endl;

        if (solver!=nullptr) {
            // call solver to optimize the graph

            if (check_cycle_dfs(*graph)) {
                std::cout<<"cycle detected before solve"<<std::endl;
                exit(10086);
            }
            graph = solver->solve(graph);
            if (check_cycle_dfs(*graph)) {
                std::cout<<"cycle detected after solve"<<std::endl;
                exit(10086);
            }
        }

        // exit(-1);


        // reset the dependencies
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            reset_dependencies(agent_id);
        }
    }

    void reset_dependencies(int agent_id) {
        auto & agent_state = agent_states[agent_id];
        if (agent_state.state_id<graph->get_num_states(agent_id)-1) {
            agent_state.dependencies.clear();
            auto && dependent_pairs = graph->get_non_switchable_in_neighbor_pairs(agent_id, agent_state.state_id+1);
            for (auto & pair: dependent_pairs ) {
                if (
                    (agent_state.dependencies.find(pair.first)==agent_state.dependencies.end() || agent_state.dependencies[pair.first]<pair.second)
                    && agent_states[pair.first].state_id<pair.second
                    ) {
                    agent_state.dependencies[pair.first] = pair.second;
                }
            }
        }
    }

    // NOTE if use this function, the graph should already be delayed. For example, using graph->delay
    void reset(
        const std::shared_ptr<Graph> & _graph,
        const std::vector<int> & curr_states
    ) {
        // since many operations are in-place, should never operate on the original graph. make a copy first.
        // note we need to deep copy here, because we need to call update curr states() and delay()
        graph=_graph->deep_copy();

        // reset states
        agent_states.clear();
        agent_states.resize(this->graph->get_num_agents());
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            int state_id = curr_states[agent_id];
            agent_states[agent_id].state_id = state_id;
            // if not reach goal, set delay
            if (state_id<graph->get_num_states(agent_id)-1) {
                int global_state_id = graph->get_global_state_id(agent_id, state_id);
                // we need to remove the basic 1-step cost
                agent_states[agent_id].delay_steps = graph->edge_manager->get_edge(global_state_id, global_state_id+1).cost - 1;
            }
        }

        // reset paths
        arrived.resize(graph->get_num_agents(), false);
        paths.resize(graph->get_num_agents(),{});
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            auto & path=(*graph->paths)[agent_id];
            auto & loc=path[agent_states[agent_id].state_id].first;
            paths[agent_id].push_back(loc);

            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {
                arrived[agent_id]=false;
            } else {
                arrived[agent_id]=true;
            }
        }

        // reset the dependencies
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            reset_dependencies(agent_id);
        }

        // if spatial delay
        if (delay_type==DelayType::UNIFORM_STATE_DELAY) {
            spatial_delays = std::make_shared<std::vector<std::vector<int> > >();
            for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
                spatial_delays->push_back({});
                for (int state_id=0;state_id<graph->get_num_states(agent_id);++state_id) {
                    if (state_id<graph->get_num_states(agent_id)-1) {
                        if (delay_distrib(rng)<delay_prob) {
                            int delay_steps=delay_steps_distrib(rng);
                            spatial_delays->back().push_back(delay_steps);
                        } else {
                            spatial_delays->back().push_back(0);
                        }
                    }
                }
            }
        }
    }

    // return cost
    int simulate(
        const std::shared_ptr<Graph> & _graph
    ) {
        std::vector<int> delays(_graph->get_num_agents(), 0);
        return simulate(_graph, *(_graph->curr_states));
    }

    // return cost
    int simulate(
        const std::shared_ptr<Graph> & _graph,
        const std::vector<int> & curr_states
    ) {
        if (!_graph->is_fixed()) {
            std::cout<<"The graph should be fixed before simulation."<<std::endl;
            exit(10086);
        }

        // check curr states is later than the curr states of the graph
        for (int agent_id=0;agent_id<_graph->get_num_agents();++agent_id) {
            if (curr_states[agent_id]<_graph->curr_states->at(agent_id)) {
                std::cout<<"The current state should be later than the current state of the graph."<<std::endl;
                exit(10087);
            }
        }

        reset(_graph, curr_states);

        std::cout<<"Start simulation."<<std::endl;
        int cost=0;
        step_ctr=0;
        total_delays=0;
        while (!terminated()) {
            if (all_stucked()) {
                std::cout<<"bug: all stucked"<<std::endl;
                break;
            }

            int step_cost=step();
            
            step_ctr+=1;
            if (verbose>0 && step_ctr%100==0) {
                std::cout<<"step "<<step_ctr<<": "<<get_remained_dists()<<" all delayed "<<all_delayed()<<" all stucked "<<all_stucked()<<" remained agents "<<get_remained_agents()<<std::endl;
            }

            cost+=step_cost;
        }
        return cost;
    }

    int get_remained_dists() {
        int remained_dists=0;
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {
                remained_dists+=graph->get_num_states(agent_id)-1-agent_states[agent_id].state_id;
            }
        }
        return remained_dists;
    }

    int get_remained_agents() {
        int remained_agents=0;
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {
                remained_agents+=1;
            }
        }
        return remained_agents;
    }

    bool all_delayed() {
        if (terminated()) {
            return false;
        }

        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {
                if (agent_states[agent_id].delay_steps==0) {
                    return false;
                }
            }
        }
        return true;
    }

    bool all_stucked() {
        if (terminated()) {
            return false;
        }

        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {
                if (agent_states[agent_id].dependencies.size()==0) {
                    return false;
                }
            }
        }
        return true;
    }

    bool simulate_delays(std::vector<int> & delays) {
        bool delayed=false;
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {

                if (delay_type==DelayType::UNIFORM_STATE_DELAY) {
                    if (spatial_delays->at(agent_id)[agent_states[agent_id].state_id]>0) {
                        delays[agent_id]=spatial_delays->at(agent_id)[agent_states[agent_id].state_id];
                        if (verbose>0) {
                            std::cout<<"step "<<step_ctr<<": agent "<<agent_id<<" delay for "<<delays[agent_id]<<" steps"<<std::endl;
                        }
                        delayed=true;
                        // NOTE: we change the system states here, but we also need to relect delays in the planning graph 
                        agent_states[agent_id].delay_steps+=delays[agent_id];
                        // only use once
                        spatial_delays->at(agent_id)[agent_states[agent_id].state_id]=0;
                    }
                } else if (delay_type==DelayType::UNIFORM_TEMPORAL_DELAY) {
                    if (delay_distrib(rng)<delay_prob) {
                        int delay_steps=delay_steps_distrib(rng);
                        delays[agent_id]=delay_steps;
                        if (verbose>0) {
                            std::cout<<"step "<<step_ctr<<": agent "<<agent_id<<" delay for "<<delay_steps<<" steps"<<std::endl;
                        }
                        delayed=true;
                        // NOTE: we change the system states here, but we also need to relect delays in the planning graph 
                        agent_states[agent_id].delay_steps+=delay_steps;
                    }
                } else if (delay_type==DelayType::DURATION_SUBSET_DELAY) {
                    if (step_ctr%delay_duration==0) {
                        if (delay_distrib(rng)<delay_subset_portion) {
                            int delay_steps=delay_steps_distrib(rng);
                            delays[agent_id]=delay_steps;
                            if (verbose>0) {
                                std::cout<<"step "<<step_ctr<<": agent "<<agent_id<<" delay for "<<delay_steps<<" steps"<<std::endl;
                            }
                            delayed=true;
                            // NOTE: we change the system states here, but we also need to relect delays in the planning graph 
                            agent_states[agent_id].delay_steps+=delay_steps;
                        }
                    }
                }
            }
        }
        return delayed;
    }

    // return cost
    int step() {
        std::vector<Location> old_locations;
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            auto & path=(*graph->paths)[agent_id];
            auto & loc=path[agent_states[agent_id].state_id].first;
            old_locations.emplace_back(loc.first, loc.second);
        }

        int cost=0;
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            // For those agents haven't reached their goals, this step will take 1 cost anyway.
            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {
                ++cost;
            }
        }


        // simulate delays
        if (delay_prob>0) {
            std::vector<int> delays(graph->get_num_agents(),0);
            bool delayed = simulate_delays(delays);

                    // debug dependencies
        // for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
        //     auto & agent_state = agent_states[agent_id];
        //     std::cout<<"agent "<<agent_id<<" state "<<agent_state.state_id<<" delay "<<agent_state.delay_steps<<" dependencies: ";
        //     for (auto & p: agent_state.dependencies) {
        //         std::cout<<"("<<p.first<<", "<<p.second<<") ";
        //     }
        //     std::cout<<std::endl;
        // }
        // // exit(-1);

            // if delayed, update the graph and dependencies
            if (delayed) {
                for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
                    total_delays+=delays[agent_id];
                }

                // std::cout<<"delayed"<<std::endl;
                update_graph(delays);
            }
        }

        // execute
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            
            auto & agent_state = agent_states[agent_id];
            
            if (agent_state.state_id>=graph->get_num_states(agent_id)-1) {
                continue;
            }
            
            if (agent_state.delay_steps==0 && agent_state.dependencies.size()==0) {
                // execute the edge
                agent_state.state_id+=1;

                // if not reach its own goal, update its own dependencies
                reset_dependencies(agent_id);
            }

            // reduce one delay step
            if (agent_state.delay_steps>0) {
                agent_state.delay_steps-=1;
            }
        }

        // update dependencies for all agents
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            auto & agent_state = agent_states[agent_id];
            for (auto it=agent_state.dependencies.begin();it!=agent_state.dependencies.end();) {
                if (agent_states[it->first].state_id>=it->second) {
                    it = agent_state.dependencies.erase(it);
                } else {
                    ++it;
                }
            }
        }

        std::vector<Location> new_locations;
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            auto & path=(*graph->paths)[agent_id];
            auto & loc=path[agent_states[agent_id].state_id].first;
            new_locations.emplace_back(loc.first, loc.second);
        }

        // check
        for (int i=0;i<graph->get_num_agents();++i) {
            for (int j=i+1;j<graph->get_num_agents();++j) {
                if (new_locations[i]==new_locations[j]) {
                    std::cout<<"vertex collision detected"<<std::endl;
                    exit(10086);
                }
            }
        }

        for (int i=0;i<graph->get_num_agents();++i) {
            for (int j=0;j<graph->get_num_agents();++j) {
                if (i!=j && new_locations[i]==old_locations[j]) {
                    std::cout<<"following collision detected"<<std::endl;
                    exit(10087);
                }
            }
        }

        // update paths
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            if (!arrived[agent_id]) {
                auto & path=(*graph->paths)[agent_id];
                auto & loc=path[agent_states[agent_id].state_id].first;
                paths[agent_id].push_back(loc);
            }

            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {
                arrived[agent_id]=false;
            } else {
                arrived[agent_id]=true;
            }
        }

        return cost;
    }

    bool terminated() {
        for (int agent_id=0;agent_id<graph->get_num_agents();++agent_id) {
            if (agent_states[agent_id].state_id<graph->get_num_states(agent_id)-1) {
                return false;
            }
        }
        return true;
    }

    int get_total_delays() {
        return total_delays;
    }

};