#include "graph.h"

using namespace std;

Graph new_graph(int n){
    set<int>* type1GInNeighbors = new set<int>[n];
    set<int>* type1OutNeighbors = new set<int>[n];
    subGraph type1G = make_tuple(type1OutNeighbors,type1GInNeighbors);

    set<int>* type2NonSwitchableGInNeighbors = new set<int>[n];
    set<int>* type2NonSwitchableGOutNeighbors = new set<int>[n];
    subGraph type2NonSwitchableG = make_tuple(type2NonSwitchableGOutNeighbors, type2NonSwitchableGInNeighbors);

    set<int>* type2SwitchableGInNeighbors = new set<int>[n];
    set<int>* type2SwitchableGOutNeighbors = new set<int>[n];
    subGraph type2SwitchableG = make_tuple(type2SwitchableGOutNeighbors, type2SwitchableGInNeighbors);

    Graph graph = make_tuple(type1G, type2NonSwitchableG, type2SwitchableG, n);
    return graph;
}

void set_edge(Graph graph, int v1, int v2, edgeType e){
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);
    int m = get<2>(graph);

    if (v1 <= 0 || v1 > n){
        return;
    }

    if (v2 <= 0 || v2 > m){
        return;
    }

    matrix[v1][v2] = e;
    return;
}

void rem_edge(Graph graph, int v1, int v2){
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);
    int m = get<2>(graph);

    if (v1 <= 0 || v1 > n){
        return;
    }

    if (v2 <= 0 || v2 > m){
        return;
    }

    matrix[v1][v2] = NULL_EDGE;
    return;
}

edgeType get_edge(Graph graph, int v1, int v2){
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);
    int m = get<2>(graph);

    if (v1 <= 0 || v1 > n){
        throw std::invalid_argument( "invalid index" );
    }

    if (v2 <= 0 || v2 > m){
        throw std::invalid_argument( "invalid index" );
    }

    return matrix[v1][v2];
}

vector<int> get_inNeighbors(Graph graph, int v) {
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);
    vector<int> neighbors;

    for (int v0 = 0; v0 < n; v0 ++) {
        if (matrix[v0][v] != NULL_EDGE) neighbors.push_back(v0);
    }
    return neighbors;
}

vector<int> get_outNeighbors(Graph graph, int v) {
    edgeType** matrix = get<0>(graph);
    int m = get<2>(graph);
    vector<int> neighbors;

    for (int v0 = 0; v0 < m; v0 ++) {
        if (matrix[v][v0] != NULL_EDGE) neighbors.push_back(v0);
    }
    return neighbors;
}

vector<int> get_type2_inNeib(Graph graph, int v) {
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);
    vector<int> neighbors;

    for (int v0 = 0; v0 < n; v0 ++) {
        if (matrix[v0][v] == TYPE2_EDGE) neighbors.push_back(v0);
    }
    return neighbors;
}

vector<int> get_type2_outNeib(Graph graph, int v) {
    edgeType** matrix = get<0>(graph);
    int m = get<2>(graph);
    vector<int> neighbors;

    for (int v0 = 0; v0 < m; v0 ++) {
        if (matrix[v][v0] == TYPE2_EDGE) neighbors.push_back(v0);
    }
    return neighbors;
}

void free_graph(Graph graph){
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);

    for (int i = 0; i < n; i++){
        delete[] matrix[i];
    }

    delete[] matrix;
}