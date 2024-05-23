#pragma once
#include "graph/graph.h"
#include "define.h"

bool check_cycle_dfs(Graph & graph, int start);

bool check_cycle_dfs(Graph & graph, vector<int>& starts);

sortResult topologicalSort(Graph & graph, sortResult state, vector<int> & starts, int u, int v);

shared_ptr<vector<COST_TYPE> > compute_longest_paths(
    const shared_ptr<vector<COST_TYPE> > & old_longest_path_lengths_ptr, 
    const shared_ptr<Graph> & graph, 
    vector<std::pair<int,int> > & fixed_edges,
    bool incremental
    );

shared_ptr<vector<shared_ptr<map<int,COST_TYPE> > > > compute_reverse_longest_paths(
    const shared_ptr<vector<shared_ptr<map<int,COST_TYPE> > > > & old_reverse_longest_path_lengths_ptr, 
    const shared_ptr<vector<COST_TYPE> > & longest_path_lengths_ptr, // we use the new longest_path_lengths, which is essentially the topoligical order for efficient udpate.
    const shared_ptr<Graph> & graph, 
    vector<pair<int,int> > & fixed_edges,
    bool incremental
    );

bool exist_this_cycle(Graph & graph, vector<pair<int,int> > & states);