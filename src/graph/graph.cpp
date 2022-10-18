#include "graph.h"

using namespace std;


bool error_check_node_range(Graph graph, int n1, int n2){
    int graph_size = get<3>(graph);

    if(n1 < 0 || n1 > graph_size){
        //throw invalid_argument("rem_type1_edge invalid n1");
        return false;
    }

    if(n2 < 0 || n2 > graph_size){
        //throw invalid_argument("rem_type1_edge invalid n2");
        return false;
    }

    return true;
}

Graph new_graph(int n){
    set<int>* type1GInNeighbors = new set<int>[n];
    set<int>* type1OutNeighbors = new set<int>[n];
    subGraph type1G = make_pair(type1OutNeighbors,type1GInNeighbors);

    set<int>* type2NonSwitchableGInNeighbors = new set<int>[n];
    set<int>* type2NonSwitchableGOutNeighbors = new set<int>[n];
    subGraph type2NonSwitchableG = make_pair(type2NonSwitchableGOutNeighbors, type2NonSwitchableGInNeighbors);

    set<int>* type2SwitchableGInNeighbors = new set<int>[n];
    set<int>* type2SwitchableGOutNeighbors = new set<int>[n];
    subGraph type2SwitchableG = make_pair(type2SwitchableGOutNeighbors, type2SwitchableGInNeighbors);

    Graph graph = make_tuple(type1G, type2NonSwitchableG, type2SwitchableG, n);
    return graph;
}

void set_type1_edge(Graph graph, int n1, int n2){
    int graph_size = get<3>(graph);

    if(n1 < 0 || n1 > graph_size){
        throw invalid_argument("set_type1_edge invalid n1");
        return;
    }

    if(n2 < 0 || n2 > graph_size){
        throw invalid_argument("set_type1_edge invalid n2");
        return;
    }

    subGraph& type1G = get<0>(graph);
    type1G.first[n1].insert(n2);
    type1G.second[n2].insert(n1);

    return;
}

void set_type2_nonSwitchable_edge(Graph graph, int n1, int n2){
    int graph_size = get<3>(graph);

    if(n1 < 0 || n1 > graph_size){
        throw invalid_argument("set_type1_edge invalid n1");
        return;
    }

    if(n2 < 0 || n2 > graph_size){
        throw invalid_argument("set_type1_edge invalid n2");
        return;
    }

    subGraph& type2NSG = get<1>(graph);
    type2NSG.first[n1].insert(n2);
    type2NSG.second[n2].insert(n1);

    return;
}

void set_type2_switchable_edge(Graph graph, int n1, int n2){
    int graph_size = get<3>(graph);

    if(n1 < 0 || n1 > graph_size){
        throw invalid_argument("set_type1_edge invalid n1");
        return;
    }

    if(n2 < 0 || n2 > graph_size){
        throw invalid_argument("set_type1_edge invalid n2");
        return;
    }

    subGraph& type2SG = get<2>(graph);
    type2SG.first[n1].insert(n2);
    type2SG.second[n2].insert(n1);

    return;
}

void rem_type1_edge(Graph graph, int n1, int n2){
    int graph_size = get<3>(graph);

    if(n1 < 0 || n1 > graph_size){
        throw invalid_argument("rem_type1_edge invalid n1");
        return;
    }

    if(n2 < 0 || n2 > graph_size){
        throw invalid_argument("rem_type1_edge invalid n2");
        return;
    }

    subGraph& type1G = get<0>(graph);
    auto itr = type1G.first[n1].find(n2);
    if(itr != type1G.first[n1].end()){
        type1G.first[n1].erase(itr);
    }

    itr = type1G.second[n2].find(n1);
    if(itr != type1G.second[n2].end()){
        type1G.second[n2].erase(itr);
    }

    return;
}

void rem_type2_nonSwitchable_edge(Graph graph, int n1, int n2){
    int graph_size = get<3>(graph);

    if(n1 < 0 || n1 > graph_size){
        throw invalid_argument("rem_type1_edge invalid n1");
        return;
    }

    if(n2 < 0 || n2 > graph_size){
        throw invalid_argument("rem_type1_edge invalid n2");
        return;
    }

    subGraph& type2NSG = get<1>(graph);
    auto itr = type2NSG.first[n1].find(n2);
    if(itr != type2NSG.first[n1].end()){
        type2NSG.first[n1].erase(itr);
    }

    itr = type2NSG.second[n2].find(n1);
    if(itr != type2NSG.second[n2].end()){
        type2NSG.second[n2].erase(itr);
    }

    return;
}

void rem_type2_switchable_edge(Graph graph, int n1, int n2){
    int graph_size = get<3>(graph);

    if(n1 < 0 || n1 > graph_size){
        throw invalid_argument("rem_type1_edge invalid n1");
        return;
    }

    if(n2 < 0 || n2 > graph_size){
        throw invalid_argument("rem_type1_edge invalid n2");
        return;
    }

    subGraph& type2SG = get<2>(graph);
    auto itr = type2SG.first[n1].find(n2);
    if(itr != type2SG.first[n1].end()){
        type2SG.first[n1].erase(itr);
    }

    itr = type2SG.second[n2].find(n1);
    if(itr != type2SG.second[n2].end()){
        type2SG.second[n2].erase(itr);
    }

    return;
    
}

bool get_type1_edge(Graph graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type1G = get<0>(graph);
    auto itr = type1G.first[n1].find(n2);
    bool result = itr == type1G.first[n1].end() ? false : true;

    return result;
}

bool get_type2_nonSwitchable_edge(Graph graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type2NSG = get<1>(graph);
    auto itr = type2NSG.first[n1].find(n2);
    bool result = itr == type2NSG.first[n1].end() ? false : true;

    return result;
}

bool get_type2_switchable_edge(Graph graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type2SG = get<2>(graph);
    auto itr = type2SG.first[n1].find(n2);
    bool result = itr == type2SG.first[n1].end() ? false : true;

    return result;
}

bool get_edge(Graph graph, int n1, int n2){

    bool result = get_type1_edge(graph, n1, n2);

    if (result == false){
        result = get_type2_nonSwitchable_edge(graph, n1, n2);
        if(result == false){
            result = get_type2_switchable_edge(graph, n1, n2);
        }
    }

    return result;
}

set<int> get_nonSwitchable_outNeib(Graph graph, int n){
    subGraph& type1G = get<0>(graph);
    subGraph& type2NSG = get<1>(graph);
    set<int> result;
    result.insert(type1G.first[n].begin(), type1G.first[n].end());
    result.insert(type2NSG.first[n].begin(), type2NSG.first[n].end());

    return result;

}

set<int> get_nonSwitchable_inNeib(Graph graph, int n){
    subGraph& type1G = get<0>(graph);
    subGraph& type2NSG = get<1>(graph);
    set<int> result;
    result.insert(type1G.second[n].begin(), type1G.second[n].end());
    result.insert(type2NSG.second[n].begin(), type2NSG.second[n].end());

    return result;
}


set<int>& get_switchable_outNeib(Graph graph, int n){
    subGraph& type2SG = get<2>(graph);
    return type2SG.first[n];
}

set<int>& get_switchable_inNeib(Graph graph, int n){
    subGraph& type2SG = get<2>(graph);
    return type2SG.second[n];
}

set<int> get_outNeighbors(Graph graph, int n){
    set<int> res1 = get_nonSwitchable_outNeib(graph, n);
    set<int>& res2 = get_switchable_outNeib(graph, n);

    set<int> result;

    result.insert(res1.begin(), res1.end());
    result.insert(res2.begin(), res2.end());

    return result;
}

set<int> get_inNeighbors(Graph graph, int n){
    set<int> res1 = get_nonSwitchable_inNeib(graph, n);
    set<int>& res2 = get_switchable_inNeib(graph, n);

    set<int> result;

    result.insert(res1.begin(), res1.end());
    result.insert(res2.begin(), res2.end());

    return result;
}

Graph copy_graph(Graph graph){

    // removed until verification of tuple implementation

    return graph;


}

void free_graph(Graph graph){

    // removed until verification of tuple implementation

    return;

}

bool dfs(Graph graph){

    // removed until verification of tuple implementation

    return true;
}

