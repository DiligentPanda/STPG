#include "graph.h"

using namespace std;

Graph new_graph(int n, int m){
    edgeType** matrix = new edgeType*[n];
    for(int i = 0; i < n; i++){
        matrix[i] = new edgeType[m]();
    }

    return make_tuple(matrix, n, m);
}

void set_edge(Graph graph, int n1, int n2, edgeType e){
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);
    int m = get<2>(graph);

    if (n1 <= 0 || n1 > n){
        return;
    }

    if (n2 <= 0 || n2 > m){
        return;
    }

    matrix[n1][n2] = e;

    return;
}

void rem_edge(Graph graph, int n1, int n2){
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);
    int m = get<2>(graph);

    if (n1 <= 0 || n1 > n){
        return;
    }

    if (n2 <= 0 || n2 > m){
        return;
    }

    matrix[n1][n2] = NULL_EDGE;

    return;
}

edgeType get_edge(Graph graph, int n1, int n2){
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);
    int m = get<2>(graph);
    if (n1 <= 0 || n1 > n){
        return;
    }

    if (n2 <= 0 || n2 > m){
        return;
    }


    return matrix[n1][n2];
}

void free_graph(Graph graph, int n, int m){
    edgeType** matrix = get<0>(graph);
    int n = get<1>(graph);

    for (int i = 0; i < n; i++){
        delete[] matrix[i];
    }

    delete[] matrix;
}