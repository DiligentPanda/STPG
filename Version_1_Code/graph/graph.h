
#include <iostream>
#include <stdlib.h>
#include <new>

using namespace std;


int** new_graph(int n, int m);

void set_edge(int** graph, int n, int m, int n1, int n2);

void rem_edge(int** grpah, int n, int m, int n1, int n2);

bool get_edge(int** graph, int n, int m, int n1, int n2);

void free_graph(int** graph, int n, int m);

bool dfs(int** graph, int n, int m);

