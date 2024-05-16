#include "Algorithm/graph.h"
#include <queue>
#include <map>
#include <algorithm>

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


shared_ptr<vector<COST_TYPE> > compute_longest_paths(const shared_ptr<vector<COST_TYPE> > & old_longest_path_lengths_ptr, const shared_ptr<Graph> & graph, vector<pair<int,int> > & fixed_edges, bool incremental) {
    if (incremental) {
        // BUG(rivers): NOTE: here we start from the initial state, which is not necessary or even buggy. We should start from the current state
        if (fixed_edges.size() == 0) {
            return compute_longest_paths(old_longest_path_lengths_ptr, graph, fixed_edges, false);
        }
  
        bool no_need_to_update=true;
        for (auto & p: fixed_edges) {
            COST_TYPE edge_cost=graph->edge_manager->get_edge(p.first, p.second).cost;
            if ((*old_longest_path_lengths_ptr)[p.first]+edge_cost>(*old_longest_path_lengths_ptr)[p.second]) {
                no_need_to_update=false;
                break;
            }
        }

        if (no_need_to_update) {
            return old_longest_path_lengths_ptr;
        }

        auto longest_path_lengths_ptr=make_shared<vector<COST_TYPE> >(*old_longest_path_lengths_ptr);
        auto & longest_path_lengths = *longest_path_lengths_ptr;

        // the newly added edge from start state to end state.
        // we will update longest path lengths incrementally from the edge end_state
        // TODO(rivers): check whether it is a min heap
        std::vector<bool> visited(graph->get_num_states(), false);
        std::priority_queue<pair<COST_TYPE, int>, vector<pair<COST_TYPE, int> >, greater<pair<COST_TYPE, int> > > pq;

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
                COST_TYPE edge_cost=graph->edge_manager->get_edge(predecessor, state).cost;
                if (longest_path_lengths[predecessor]+edge_cost > longest_path_lengths[state]) {
                    longest_path_lengths[state] = longest_path_lengths[predecessor]+edge_cost;
                }
            }

            auto && successors = graph->get_out_neighbor_global_ids(state);
            for (auto successor: successors) {
                COST_TYPE edge_cost=graph->edge_manager->get_edge(state, successor).cost;
                if (!visited[successor] && longest_path_lengths[state]+edge_cost>longest_path_lengths[successor]) {
                    visited[successor] = true;
                    pq.emplace(longest_path_lengths[successor], successor);
                }
            }
        }

        return longest_path_lengths_ptr;
    } else {
        auto longest_path_lengths_ptr=make_shared<vector<COST_TYPE> >(graph->get_num_states(), 0);
        std::vector<size_t> in_degrees(graph->get_num_states(), 0);
        // TODO: we don't need visited here, because of in_degree=0 means all predecessors have been updated
        std::vector<bool> visited(graph->get_num_states(), false);
        std::queue<int> q;

        // count the indegrees of all nodes
        // TODO(rivers): we can start from current states
        for (int i=0; i<graph->get_num_states(); i++) {
            in_degrees[i] = graph->get_in_degree(i);
            if (in_degrees[i] == 0) {
                q.push(i);
                visited[i]=true;
            }
        }

        while (!q.empty()) {
            auto state=q.front();
            q.pop();
            COST_TYPE longest_lenghth=0;
            auto && predecessors = graph->get_in_neighbor_global_ids(state);
            for (auto predecessor: predecessors) {
                COST_TYPE edge_cost=graph->edge_manager->get_edge(predecessor, state).cost;
                longest_lenghth=max(longest_lenghth, (*longest_path_lengths_ptr)[predecessor]+edge_cost);
            }
            (*longest_path_lengths_ptr)[state]=longest_lenghth;

            auto && successors = graph->get_out_neighbor_global_ids(state);
            for (auto successor: successors) {
                // if not computed yet
                if (!visited[successor]) { 
                    in_degrees[successor]--;
                    if (in_degrees[successor]==0) {
                        q.push(successor);
                        visited[successor]=true;
                    }
                }
            }
        }

        return longest_path_lengths_ptr;
    }
}

shared_ptr<vector<shared_ptr<map<int,COST_TYPE> > > > compute_reverse_longest_paths(
    const shared_ptr<vector<shared_ptr<map<int,COST_TYPE> > > > & old_reverse_longest_path_lengths_ptr, 
    const shared_ptr<vector<COST_TYPE> > & longest_path_lengths_ptr, // we use the new longest_path_lengths, which is essentially the topoligical order for efficient udpate.
    const shared_ptr<Graph> & graph, 
    vector<pair<int,int> > & fixed_edges,
    bool incremental
    ) {

    if (incremental) {
        // BUG(rivers): NOTE: here we start from the initial state, which is not necessary or even buggy. We should start from the current state
        if (fixed_edges.size() == 0) {
            return compute_reverse_longest_paths(old_reverse_longest_path_lengths_ptr, longest_path_lengths_ptr, graph, fixed_edges, false);
        }

        bool no_need_to_update=true;
        auto & old_reverse_longest_path_lengths = *old_reverse_longest_path_lengths_ptr;
        for (auto & edge: fixed_edges) {
            COST_TYPE edge_cost = graph->edge_manager->get_edge(edge.first,edge.second).cost;
            for (auto & p: *(old_reverse_longest_path_lengths[edge.second]) ) {
                if (old_reverse_longest_path_lengths[edge.first]->find(p.first)==old_reverse_longest_path_lengths[edge.first]->end() || p.second+edge_cost>(*old_reverse_longest_path_lengths[edge.first])[p.first]) {
                    no_need_to_update=false;
                    break;
                }
            }
        }

        if (no_need_to_update) {
            return old_reverse_longest_path_lengths_ptr;
        }

        // the newly added edge from start state to end state.
        // we will update longest path lengths incrementally from the edge end_state
        // TODO(rivers): check whether it is a min heap
        auto reverse_longest_path_lengths_ptr=make_shared<vector<shared_ptr<map<int, COST_TYPE> > > >(*old_reverse_longest_path_lengths_ptr);
        auto & reverse_longest_path_lengths=*reverse_longest_path_lengths_ptr;
        auto & longest_path_lengths = *longest_path_lengths_ptr;

        std::vector<bool> visited(graph->get_num_states(), false);
        // we should use a max heap for the backward propagation
        std::priority_queue<pair<COST_TYPE, int>, vector<pair<COST_TYPE, int> >, less<pair<COST_TYPE, int> > > pq;

        for (auto & p: fixed_edges) {
            visited[p.first] = true;
            pq.emplace(longest_path_lengths[p.first], p.first);
        }

        while (!pq.empty()) {
            auto top = pq.top();
            pq.pop();
            int state = top.second;

            // update the 
            auto && successors = graph->get_out_neighbor_global_ids(state);
            for (auto successor: successors) {
                COST_TYPE edge_cost = graph->edge_manager->get_edge(state, successor).cost;
                bool first=true;
                for (auto & p: *reverse_longest_path_lengths[successor]) {
                    if (reverse_longest_path_lengths[state]->find(p.first)==reverse_longest_path_lengths[state]->end() || p.second+edge_cost>(*reverse_longest_path_lengths[state])[p.first]) {
                        if (first) {
                            reverse_longest_path_lengths[state] = make_shared<map<int,COST_TYPE> >(*reverse_longest_path_lengths[state]);
                            first=false;
                        }
                        (*reverse_longest_path_lengths[state])[p.first] = p.second+edge_cost;
                    }
                }
            }

            auto && predecessors = graph->get_in_neighbor_global_ids(state);
            for (auto predecessor: predecessors) {
                COST_TYPE edge_cost = graph->edge_manager->get_edge(predecessor, state).cost;
                if (!visited[predecessor]) {
                    bool no_need_to_update=true;
                    for (auto & p: *(reverse_longest_path_lengths[state])) {
                        if (reverse_longest_path_lengths[predecessor]->find(p.first)==reverse_longest_path_lengths[predecessor]->end() || p.second+edge_cost>(*reverse_longest_path_lengths[predecessor])[p.first]) {
                            no_need_to_update=false;
                            break;
                        }
                    }

                    if (no_need_to_update) {
                        continue;
                    }
    
                    visited[predecessor] = true;
                    pq.emplace(longest_path_lengths[predecessor], predecessor);
                }
            }
        }

        return reverse_longest_path_lengths_ptr;
    } else {
        // state_idx-> a map { agent_id (indicating its goal state): reverse longest path length }
        auto reverse_longest_path_lengths_ptr=make_shared<vector<shared_ptr<map<int, COST_TYPE> > > >();
        auto & reverse_longest_path_lengths = *reverse_longest_path_lengths_ptr;
        for (int i=0; i<graph->get_num_states(); i++) {
            reverse_longest_path_lengths.emplace_back(make_shared<map<int, COST_TYPE> >());
        }

        std::vector<std::pair<int,int> > topo_orders;
        for (int i=0; i<graph->get_num_states(); i++) {
            topo_orders.emplace_back((*longest_path_lengths_ptr)[i], i);
        }

        std::sort(topo_orders.begin(), topo_orders.end(), std::greater<std::pair<COST_TYPE, int> >());

        // initialize for the last state
        for (int i=0;i<graph->get_num_agents(); ++i) {
            int last_state=(*graph->accum_state_cnts_end)[i]-1;
            (*reverse_longest_path_lengths[last_state])[i]=0;
        }

        for (auto &p: topo_orders) {
            int state=p.second;
            auto && successors = graph->get_out_neighbor_global_ids(state);
            // all successors must be computed
            for (auto successor: successors) {
                COST_TYPE edge_cost = graph->edge_manager->get_edge(state, successor).cost;
                for (auto & q: *(*reverse_longest_path_lengths_ptr)[successor]) {
                    // q.first: agent_id, q.second: path length
                    if (reverse_longest_path_lengths[state]->find(q.first)==reverse_longest_path_lengths[state]->end() || q.second+edge_cost>(*reverse_longest_path_lengths[state])[q.first]) {
                        (*reverse_longest_path_lengths[state])[q.first]=q.second+edge_cost;
                    }
                }
            }
        }

        return reverse_longest_path_lengths_ptr;
    }
}

bool exist_this_cycle(Graph & graph, vector<pair<int,int> > & states) {
    for (int i=0;i<(int)states.size();++i) {
        int next_idx=(i+1)%states.size();
        // check next_idx is in the graph
        auto global_id=graph.get_global_state_id(states[i].first, states[i].second);
        auto && neighbors = graph.get_out_neighbor_global_ids(global_id);
        bool found=false;
        for (auto & neighbor : neighbors) {
            auto p=graph.get_agent_state_id(neighbor);
            if (p.first==states[next_idx].first && p.second==states[next_idx].second) {
                found=true;
                break;
            }
        }
        if (!found) {
            cout<<"not found edge from "<<states[i].first<<","<<states[i].second<<" to "<<states[next_idx].first<<","<<states[next_idx].second<<endl;
            return false;
        }
    }
    return true;
}