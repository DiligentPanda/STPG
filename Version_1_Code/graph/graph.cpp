#include "graph.h"

using namespace std;

int** new_graph(int n, int m){
    int** graph = new int*[n];
    for(int i = 0; i < n; i++){
        graph[i] = new int[m]();
    }

    return graph;
}

void set_edge(int** graph, int n, int m, int n1, int n2){
    if (n1 <= 0 || n1 > n){
        return;
    }

    if (n2 <= 0 || n2 > m){
        return;
    }

    graph[n1][n2] = 1;

    return;
}

void rem_edge(int** graph, int n, int m, int n1, int n2){
    if (n1 <= 0 || n1 > n){
        return;
    }

    if (n2 <= 0 || n2 > m){
        return;
    }

    graph[n1][n2] = 0;

    return;
}

bool get_edge(int** graph, int n, int m, int n1, int n2){
    if (n1 <= 0 || n1 > n){
        return;
    }

    if (n2 <= 0 || n2 > m){
        return;
    }

    return graph[n1][n2];
}

void free_graph(int** graph, int n, int m){
    for (int i = 0; i < n; i++){
        delete[] graph[i];
    }

    delete[] graph;
}