#pragma once
#include "types.h"

bool check_cycle_dfs(Graph & graph, int start);

bool check_cycle_dfs(Graph & graph, vector<int>& starts);

sortResult topologicalSort(Graph & graph, sortResult state, vector<int> & starts, int u, int v);