#include <new>
#include <tuple>
#include <stdlib.h>

using namespace std;

// The adjacency matrix plus n and m
typedef tuple<int**, int, int> Graph;

Graph new_graph(int n, int m);

void set_edge(Graph graph, int n1, int n2);

void rem_edge(Graph grpah, int n1, int n2);

bool get_edge(Graph graph, int n1, int n2);

void free_graph(Graph graph);

bool dfs(Graph graph);

