#include "graph/graph.h"

// for receding horizon control
void Graph::make_switchable(int window) {
    if (!is_fixed()) {
        std::cout<<"Graph::make_switchable() should be called when the graph is fixed"<<std::endl;
        exit(-1);
    }

    // TODO(rivers): this is an implementation close to the algorithm 3 in the paper Receding Horizon Re-Ordering of Multi-Agent Execution Schedules
    // However, currently we still consider the overall cost, rather than only the cost within the window

    // add all states within the window
    // std::vector<int> subgraph_states;
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
    //         subgraph_states.push_back(global_state_id);
    //     }
    // }

    // // add all edges within the window
    // // TODO(rivers): we use set here for simplicity to 
    // std::set<std::pair<int,int>> subgraph_edges;
    // for (int global_state_id:subgraph_states) {
    //     auto && in_neighbors=get_in_neighbor_global_ids(global_state_id);
    //     for (int in_neighbor:in_neighbors) {
    //         if (std::find(subgraph_states.begin(),subgraph_states.end(),in_neighbor)!=subgraph_states.end()) {
    //             subgraph_edges.push_back(std::make_pair(in_neighbor,global_state_id));
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