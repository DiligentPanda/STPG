#pragma once
#include "types.h"

bool check_cycle_dfs(Graph & graph, int start);

bool check_cycle_dfs(Graph & graph, vector<int>& starts);

sortResult topologicalSort(Graph & graph, sortResult state, vector<int> & starts, int u, int v);

shared_ptr<vector<int> > compute_longest_paths(
    const shared_ptr<vector<int> > & old_longest_path_lengths_ptr, 
    const shared_ptr<Graph> & graph, 
    vector<std::pair<int,int> > & fixed_edges
    );

shared_ptr<vector<shared_ptr<map<int,int> > > > compute_reverse_longest_paths(
    const shared_ptr<vector<shared_ptr<map<int,int> > > > & old_reverse_longest_path_lengths_ptr, 
    const shared_ptr<vector<int> > & longest_path_lengths_ptr, // we use the new longest_path_lengths, which is essentially the topoligical order for efficient udpate.
    const shared_ptr<Graph> & graph, 
    vector<pair<int,int> > & fixed_edges
    );