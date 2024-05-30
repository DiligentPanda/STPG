#include "graph/graph.h"
#include <set>
#include "group/group.h"

// for receding horizon control
void Graph::make_switchable(int window, std::shared_ptr<GroupManager> group_manager) {
    if (!is_fixed()) {
        std::cout<<"Graph::make_switchable() should be called when the graph is fixed"<<std::endl;
        exit(-1);
    }

    if (group_manager==nullptr) {
        std::cout<<"Graph::make_switchable() in the window version should be called with a group manager"<<std::endl;
        exit(-1);
    }

    // we will use the first version window planning here.
    make_switchable();

    // get valid edge group ids
    std::set<int> valid_group_ids;
    for (int agent_id=0;agent_id<get_num_agents();++agent_id) {
        int curr_state=(*curr_states)[agent_id];
        int num_states=get_num_states(agent_id);
        for (int j=0;j<window;++j) {
            int state=curr_state+j;
            if (state>=num_states) {
                break;
            }
            int global_state_id=get_global_state_id(agent_id,state);
            auto & in_neighbors=switchable_type2_edges->get_in_neighbor_global_ids(global_state_id);
            for (int in_neighbor:in_neighbors) {
                valid_group_ids.insert(group_manager->get_group_id(in_neighbor,global_state_id));
            }
            auto & out_neighbors=switchable_type2_edges->get_out_neighbor_global_ids(global_state_id);
            for (int out_neighbor:out_neighbors) {
                valid_group_ids.insert(group_manager->get_group_id(global_state_id,out_neighbor));
            }
        }
    }

    // fix edges beyond window
    for (int agent_id=0;agent_id<get_num_agents();++agent_id) {
        int curr_state=(*curr_states)[agent_id];
        int num_states=get_num_states(agent_id);
        for (int state_id=curr_state;state_id<num_states;++state_id) {
            int global_state_id=get_global_state_id(agent_id,state_id);
            auto in_neighbors=switchable_type2_edges->get_in_neighbor_global_ids(global_state_id);
            // we need make a copy here, because it is changing ?
            for (int in_neighbor:in_neighbors) {
                // if not in the valid group: fix it
                if (valid_group_ids.find(group_manager->get_group_id(in_neighbor,global_state_id))==valid_group_ids.end()) {
                    fix_switchable_type2_edge(in_neighbor,global_state_id);
                }
            }
        }
    }

    // TODO(rivers): this is an implementation close to the algorithm 3 in the paper Receding Horizon Re-Ordering of Multi-Agent Execution Schedules
    // However, currently we still consider the overall cost, rather than only the cost within the window

    // add all states within the window
    // std::set<int> subgraph_states;
    // int num_agents=get_num_agents();
    // for (int agent_id=0;agent_id<num_agents;++agent_id) {
    //     int curr_state=(*curr_states)[agent_id];
    //     int num_states=get_num_states(agent_id);
    //     for (int j=0;j<window;++j) {
    //         int state=(*curr_states)[agent_id]+j;
    //         if (state>=num_states) {
    //             break;
    //         }
    //         int global_state_id=get_global_state_id(agent_id,state);
    //         subgraph_states.insert(global_state_id);
    //     }
    // }

    // // add all edges within the window
    // // TODO(rivers): we use set here for simplicity to 
    // std::set<std::pair<int,int>> subgraph_edges;
    // for (int global_state_id:subgraph_states) {
    //     auto && in_neighbors=get_in_neighbor_global_ids(global_state_id);
    //     for (int in_neighbor:in_neighbors) {
    //         if (std::find(subgraph_states.begin(),subgraph_states.end(),in_neighbor)!=subgraph_states.end()) {
    //             subgraph_edges.emplace(in_neighbor,global_state_id);
    //         }
    //     }
        

    // }








    // we don't want to optimize anything before curr_states
    // we always maintain edges to match the curr states, so we actually don't need to call this again.
    // update_curr_states(*curr_states);

    // swap switchable and non-switchable edges
    // now 
    // auto ptr=switchable_type2_edges;
    // switchable_type2_edges=non_switchable_type2_edges;
    // non_switchable_type2_edges=ptr;
    
    // fix some edges that are not switchable
    // fix_edges_from_next_states(*curr_states);
    // fix_edges_to_last_states();
}