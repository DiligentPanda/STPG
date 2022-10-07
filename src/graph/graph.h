#include <new>
#include <stdexcept>

#include "../types.h"

using namespace std;

Graph new_graph(int n, int m);

void set_edge(Graph graph, int n1, int n2, edgeType e);

void rem_edge(Graph graph, int n1, int n2);

edgeType get_edge(Graph graph, int n1, int n2);

void free_graph(Graph graph);

bool dfs(Graph graph);

