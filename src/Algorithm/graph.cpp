#include "Algorithm/graph.h"
#include <queue>

bool isCyclicUtil(Graph &graph, int v, vector<bool> &visited,
                         vector<bool> &recStack)
{
    if (visited[v] == false) {
        visited[v] = true;
        recStack[v] = true;
 
        auto && neighbors = graph.get_out_neighbor_global_ids(v);
        for (auto i = neighbors.begin(); i != neighbors.end(); i++){
            if (!visited[*i]
                && isCyclicUtil(graph, *i, visited, recStack))
                return true;
            else if (recStack[*i])
                return true;
        }
    }
 
    recStack[v] = false;
    return false;
}

bool check_cycle_dfs(Graph& graph, int start) {
    int graph_size = graph.get_num_states();
    vector<bool> visited (graph_size, false);
    vector<bool> recStack (graph_size, false);

    return isCyclicUtil(graph, start, visited, recStack);
}

bool check_cycle_dfs(Graph& graph, vector<int>& starts) {
  int graph_size = graph.get_num_states();
  vector<bool> visited (graph_size, false);
  vector<bool> recStack (graph_size, false);
  
  for (auto start: starts) {
    if (!visited[start]) {
      if (isCyclicUtil(graph, start, visited, recStack)) {
        return true;
      }
    }
  }
  return false;
}

void build_time_arr(Graph & graph, vector<bool> & visited, shared_ptr<vector<int> > sorted_vertecies, shared_ptr<vector<int> > sorted_times, int current, int & time) {
    if(visited[current] == true){
        // revisit
        return;
    }

    // visit
    visited[current] = true;

    // assign time to all successors first
    auto && neighbors = graph.get_out_neighbor_global_ids(current);
    for(auto itr = neighbors.begin(); itr != neighbors.end(); itr++){
        build_time_arr(graph, visited, sorted_vertecies, sorted_times, *itr, time);
    }

    //finish
    // (*state).push_back(current);
    (*sorted_vertecies)[time] = current;
    (*sorted_times)[current] = time;
    time--;
}

/* (incremental?) topological sort

COMMENT(rivers): this is a recursive implementation, which might not be necessary. We can just count the in-/out-degree.
TODO(rivers): it seems that the incremental version has not been implemented yet.

input:
    sortResult: (init) sorted restuls, including the following two arrays of the same size.
        topological order -> vertex global idx
        vertex global idx -> topological order
    agent_starts: current agent vertex local idxs
    u: ?
    v: ?
return:
    sortResult: see input sortResult
*/ 
sortResult topologicalSort(Graph & graph, sortResult state, vector<int> & starts, int u, int v) {
    int graph_size = graph.get_num_states();
  
    shared_ptr<vector<int> > time_arr = state.first;
    shared_ptr<vector<int> > vertex_arr = state.second;

    if(time_arr == nullptr && vertex_arr == nullptr) {
        auto sorted_vertecies = make_shared<vector<int> > (graph_size, -1);
        auto sorted_times = make_shared<vector<int> > (graph_size, -1);
        vector<bool> visited(graph_size, false);
        int time = graph_size - 1;

        int n = starts.size();
        for(int i = 0; i < n; i++){
            build_time_arr(graph, visited, sorted_vertecies, sorted_times, starts[i], time);
        }

        sortResult ret_val = make_pair(sorted_vertecies, sorted_times);
        return ret_val;
    }
    else{
        // Error scenario
        sortResult ret_val = make_pair(nullptr, nullptr);
        return ret_val;
    }

    
}


shared_ptr<vector<int> > compute_longest_paths(const shared_ptr<vector<int> > & old_longest_path_lengths_ptr, const shared_ptr<Graph> & graph, vector<pair<int,int> > & fixed_edges) {
    // BUG(rivers): NOTE: here we start from the initial state, which is not necessary or even buggy. We should start from the current state
    if (fixed_edges.size() == 0) {
        // init case: there is only type 1 edges, which is easy
        auto & longest_path_length = *old_longest_path_lengths_ptr;
        for (int agent_id=0; agent_id<graph->get_num_agents(); agent_id++) {
            int num_states = graph->get_num_states(agent_id);
            for (int state_id=0; state_id<num_states; state_id++) {
                int global_state_id=graph->get_global_state_id(agent_id, state_id);
                longest_path_length[global_state_id] = state_id;
            }
        }
        return old_longest_path_lengths_ptr;
    }

    bool no_need_to_update=true;
    for (auto & p: fixed_edges) {
        if ((*old_longest_path_lengths_ptr)[p.first]+1>(*old_longest_path_lengths_ptr)[p.second]) {
            no_need_to_update=false;
            break;
        }
    }

    if (no_need_to_update) {
        return old_longest_path_lengths_ptr;
    }

    auto longest_path_lengths_ptr=make_shared<vector<int> >(*old_longest_path_lengths_ptr);
    auto & longest_path_lengths = *longest_path_lengths_ptr;

    // the newly added edge from start state to end state.
    // we will update longest path lengths incrementally from the edge end_state
    // TODO(rivers): check whether it is a min heap
    std::vector<bool> visited(graph->get_num_states(), false);
    std::priority_queue<pair<int, int>, vector<pair<int, int> >, greater<pair<int, int> > > pq;

    for (auto & p: fixed_edges) {
        visited[p.second] = true;
        pq.emplace(longest_path_lengths[p.second], p.second);
    }

    while (!pq.empty()) {
        auto top = pq.top();
        pq.pop();
        int state = top.second;

        // update the 
        auto && predecessors = graph->get_in_neighbor_global_ids(state);
        for (auto predecessor: predecessors) {

            if (longest_path_lengths[predecessor]+1>=longest_path_lengths[state]) {
                longest_path_lengths[state] = longest_path_lengths[predecessor]+1;
            }
        }

        auto && successors = graph->get_out_neighbor_global_ids(state);
        for (auto successor: successors) {
            if (!visited[successor] && longest_path_lengths[state]+1>longest_path_lengths[successor]) {
                visited[successor] = true;
                pq.emplace(longest_path_lengths[successor], successor);
            }
        }
    }

    return longest_path_lengths_ptr;
}