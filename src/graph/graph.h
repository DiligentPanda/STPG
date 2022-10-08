#include <new>
#include <stdexcept>

#include "../types.h"

using namespace std;

Graph new_graph(int n, int m);

void set_type1_edge(Graph graph, int n1, int n2);

void set_type2_nonSwitchable_edge(Graph graph, int n1, int n2);

void set_type2_switchable_edge(Graph graph, int n1, int n2);

// Cannot imagine when we'll need this but still have it here
void rem_type1_edge(Graph graph, int v1, int v2);

void rem_type2_nonSwitchable_edge(Graph graph, int v1, int v2);

void rem_type2_switchable_edge(Graph graph, int v1, int v2);

bool get_type1_edge(Graph graph, int v1, int v2);

bool get_type2_nonSwitchable_edge(Graph graph, int v1, int v2);

bool get_type2_switchable_edge(Graph graph, int v1, int v2);

bool get_edge(Graph graph, int v1, int v2);

vector<int> get_inNeighbors(Graph graph, int v);

vector<int> get_outNeighbors(Graph graph, int v);

vector<int> get_nonSwitchable_inNeib(Graph graph, int v);

vector<int> get_nonSwitchable_outNeib(Graph graph, int v);

vector<int> get_switchable_inNeib(Graph graph, int v);

vector<int> get_switchable_outNeib(Graph graph, int v);

Graph copy_graph(Graph graph);

void free_graph(Graph graph);

bool dfs(Graph graph);
