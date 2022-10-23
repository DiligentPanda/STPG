#include <stack>
#include <algorithm>
#include "graph.h"

using namespace std;


bool error_check_node_range(Graph& graph, int n1, int n2){
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

void set_type1_edge(Graph& graph, int n1, int n2){
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

void set_type2_nonSwitchable_edge(Graph& graph, int n1, int n2){
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

void set_type2_switchable_edge(Graph& graph, int n1, int n2){
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

void rem_type1_edge(Graph& graph, int n1, int n2){
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

void rem_type2_nonSwitchable_edge(Graph& graph, int n1, int n2){
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

void rem_type2_nonSwitchable_neighborhood(Graph& graph, int n){
    int graph_size = get<3>(graph);

    if(n < 0 || n > graph_size){
        throw invalid_argument("rem_type1_edge invalid n1");
        return;
    }

    subGraph& type2NSG = get<1>(graph);
    type2NSG.first[n].clear();  

    return;
}

void rem_type2_switchable_edge(Graph& graph, int n1, int n2){
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

bool get_type1_edge(Graph& graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type1G = get<0>(graph);
    auto itr = type1G.first[n1].find(n2);
    bool result = itr == type1G.first[n1].end() ? false : true;

    return result;
}

bool get_type2_nonSwitchable_edge(Graph& graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type2NSG = get<1>(graph);
    auto itr = type2NSG.first[n1].find(n2);
    bool result = itr == type2NSG.first[n1].end() ? false : true;

    return result;
}

bool get_type2_switchable_edge(Graph& graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type2SG = get<2>(graph);
    auto itr = type2SG.first[n1].find(n2);
    bool result = itr == type2SG.first[n1].end() ? false : true;

    return result;
}

bool get_edge(Graph& graph, int n1, int n2){

    bool result = get_type1_edge(graph, n1, n2);

    if (result == false){
        result = get_type2_nonSwitchable_edge(graph, n1, n2);
        if(result == false){
            result = get_type2_switchable_edge(graph, n1, n2);
        }
    }

    return result;
}

set<int> get_nonSwitchable_outNeib(Graph& graph, int n){
    subGraph& type1G = get<0>(graph);
    subGraph& type2NSG = get<1>(graph);
    set<int> result;
    result.insert(type1G.first[n].begin(), type1G.first[n].end());
    result.insert(type2NSG.first[n].begin(), type2NSG.first[n].end());

    return result;

}

set<int> get_nonSwitchable_inNeib(Graph& graph, int n){
    subGraph& type1G = get<0>(graph);
    subGraph& type2NSG = get<1>(graph);
    set<int> result;
    result.insert(type1G.second[n].begin(), type1G.second[n].end());
    result.insert(type2NSG.second[n].begin(), type2NSG.second[n].end());

    return result;
}

set<int>& get_switchable_outNeib(Graph& graph, int n){
    subGraph& type2SG = get<2>(graph);
    return type2SG.first[n];
}

set<int>& get_switchable_inNeib(Graph& graph, int n){
    subGraph& type2SG = get<2>(graph);
    return type2SG.second[n];
}

set<int>& get_type2_nonSwitchable_inNeib(Graph& graph, int n) {
    return get<1>(graph).second[n];
}

set<int> get_outNeighbors(Graph& graph, int n){
    set<int> res1 = get_nonSwitchable_outNeib(graph, n);
    set<int>& res2 = get_switchable_outNeib(graph, n);

    set<int> result;

    result.insert(res1.begin(), res1.end());
    result.insert(res2.begin(), res2.end());

    return result;
}

set<int> get_inNeighbors(Graph& graph, int n){
    set<int> res1 = get_nonSwitchable_inNeib(graph, n);
    set<int>& res2 = get_switchable_inNeib(graph, n);

    set<int> result;

    result.insert(res1.begin(), res1.end());
    result.insert(res2.begin(), res2.end());

    return result;
}

void set_switchable_nonSwitchable(Graph& graph){
    int graph_size = get<3>(graph);
    subGraph& graph2NS = get<1>(graph);
    subGraph& graph2S = get<2>(graph);
    for(int i = 0; i < graph_size; i++){
        for(auto itr = graph2S.first[i].begin(); itr != graph2S.first[i].end(); itr++){
            graph2NS.first[i].insert(*itr);
        }
        graph2S.first[i].clear();
        for(auto itr = graph2S.second[i].begin(); itr != graph2S.second[i].end(); itr++){
            graph2NS.second[i].insert(*itr);
        }
        graph2S.second[i].clear();
    }

    return;
}

Graph copy_graph(Graph& graph){
    int n = get<3>(graph);

    /*set<int>* type1GInNeighbors = new set<int>[n];
    for(int i = 0; i < n; i++){
        copy(get<0>(graph).second[i].begin(), get<0>(graph).second[i].end(), inserter(type1GInNeighbors[i], type1GInNeighbors[i].begin()));
    }

    set<int>* type1OutNeighbors = new set<int>[n];
    for(int i = 0; i < n; i++){
        copy(get<0>(graph).first[i].begin(), get<0>(graph).first[i].end(), inserter(type1OutNeighbors[i], type1OutNeighbors[i].begin()));
    }*/
    //subGraph type1G = make_pair(type1OutNeighbors,type1GInNeighbors);
    subGraph type1G = make_pair(get<0>(graph).first, get<0>(graph).second);

    set<int>* type2NonSwitchableGInNeighbors = new set<int>[n];
    for(int i = 0; i < n; i++){
        copy(get<1>(graph).second[i].begin(), get<1>(graph).second[i].end(), inserter(type2NonSwitchableGInNeighbors[i], type2NonSwitchableGInNeighbors[i].begin()));
    }
    set<int>* type2NonSwitchableGOutNeighbors = new set<int>[n];
    for(int i = 0; i < n; i++){
        copy(get<1>(graph).first[i].begin(), get<1>(graph).first[i].end(), inserter(type2NonSwitchableGOutNeighbors[i], type2NonSwitchableGOutNeighbors[i].begin()));
    }
    subGraph type2NonSwitchableG = make_pair(type2NonSwitchableGOutNeighbors, type2NonSwitchableGInNeighbors);

    set<int>* type2SwitchableGInNeighbors = new set<int>[n];
    for(int i = 0; i < n; i++){
        copy(get<2>(graph).second[i].begin(), get<2>(graph).second[i].end(), inserter(type2SwitchableGInNeighbors[i], type2SwitchableGInNeighbors[i].begin()));
    }
    set<int>* type2SwitchableGOutNeighbors = new set<int>[n];
    for(int i = 0; i < n; i++){
        copy(get<2>(graph).first[i].begin(), get<2>(graph).first[i].end(), inserter(type2SwitchableGOutNeighbors[i], type2SwitchableGOutNeighbors[i].begin()));
    }
    subGraph type2SwitchableG = make_pair(type2SwitchableGOutNeighbors, type2SwitchableGInNeighbors);

    Graph graphC = make_tuple(type1G, type2NonSwitchableG, type2SwitchableG, n);
    return graphC;

}

void free_graph(Graph& graph){
    //delete[] get<0>(graph).first;
    //delete[] get<0>(graph).second;

    delete[] get<1>(graph).first;
    delete[] get<1>(graph).second;

    delete[] get<2>(graph).first;
    delete[] get<2>(graph).second;

    return;
}

void print_graph(Graph& graph){
    
    subGraph& type1G = get<0>(graph);
    subGraph& type2NSG = get<1>(graph);
    subGraph& type2SG = get<2>(graph);
    int size = get<3>(graph);

    cout<<"Printing Graph"<<endl;
    cout<<"Type 1 Graph\n"<<endl;
    
    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type1G.first[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type1G.second[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;

    cout<<"Type 2 Non-Switchable Graph\n"<<endl;

    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2NSG.first[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2NSG.second[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;

    cout<<"Type 2 Switchable Graph\n"<<endl;

    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2SG.first[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2SG.second[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
    
    cout<<endl;
    return;
}

void print_graph_concise(Graph& graph){
    subGraph& type1G = get<0>(graph);
    subGraph& type2NSG = get<1>(graph);
    subGraph& type2SG = get<2>(graph);
    int size = get<3>(graph);

    cout<<"Printing Graph"<<endl;
    cout<<"Type 1 Graph\n"<<endl;
    
    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type1G.first[i];
        if(g.size() == 0){
            continue;
        }
        cout<<"1ON: "<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type1G.second[i];
        if(g.size() == 0){
            continue;
        }
        cout<<"1ON: "<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;

    cout<<"Type 2 Non-Switchable Graph\n"<<endl;

    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2NSG.first[i];
        if(g.size() == 0){
            continue;
        }
        cout<<"2NSON: "<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2NSG.second[i];
        if(g.size() == 0){
            continue;
        }
        cout<<"2NSIN: "<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;

    cout<<"Type 2 Switchable Graph\n"<<endl;

    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2SG.first[i];
        if(g.size() == 0){
            continue;
        }
        cout<<"SON: "<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2SG.second[i];
        if(g.size() == 0){
            continue;
        }
        cout<<"SIN: "<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
    
    cout<<endl;
    return;
}

void print_graph_s2(Graph& graph){
    subGraph& type2SG = get<2>(graph);
    int size = get<3>(graph);

    cout<<"Printing Graph"<<endl;

    cout<<"Type 2 Switchable Graph\n"<<endl;

    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2SG.first[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2SG.second[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
    
    return;
}

void print_graph_n2(Graph& graph){
    
    subGraph& type2NSG = get<1>(graph);
    int size = get<3>(graph);

    cout<<"Printing Graph"<<endl;

    cout<<"Type 2 Non-Switchable Graph\n"<<endl;

    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2NSG.first[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = type2NSG.second[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
    return;
}

bool check_cycle_NS_helper(Graph& graph, int current, vector<bool>& visited, vector<bool>& parents, bool type2_flag, int flag_parent){
    //if(visited[current] == false){
    visited[current] = true;
    parents[current] = true;


    set<int> neighborhood1 = get<0>(graph).first[current];
    set<int> neighborhood2NS = get<1>(graph).first[current];
    
    if (type2_flag){
        set<int> reversible_edge = get<0>(graph).second[current];
        auto itr = reversible_edge.begin();
        if(*itr != flag_parent && parents[*itr] == true){
            return true;
        }
        bool result = *itr != flag_parent ? check_cycle_NS_helper(graph, *itr, visited, parents, false, current) : false;
        if(result == true){
            return true;
        }
    }

    // Type 1
    for(auto itr = neighborhood1.begin(); itr != neighborhood1.end(); itr++){
        if(*itr != flag_parent && parents[*itr] == true){
            return true;
        }
        bool result = *itr != flag_parent ? check_cycle_NS_helper(graph, *itr, visited, parents, false, -1) : false;
        if(result == true){
            return true;
        }
    }

    // Type 2 NS
    for(auto itr = neighborhood2NS.begin(); itr != neighborhood2NS.end(); itr++){
        if(*itr != flag_parent && parents[*itr] == true){
            return true;
        }
        bool result = *itr != flag_parent ? check_cycle_NS_helper(graph, *itr, visited, parents, true, -1) : false;
        if(result == true){
            return true;
        } 
    }

    parents[current] = false;
    //}
    return false;
}

bool check_cycle_nonSwitchable(Graph& graph, int start){
    int graph_size = get<3>(graph);
    vector<bool> visited (graph_size, false);
    vector<bool> parents (graph_size, false);

    return check_cycle_NS_helper(graph, start, visited, parents, false, -1);
}

bool check_cycle_NS_helper_old(Graph& graph, int current, vector<bool>& visited, vector<bool>& parents){
    if(visited[current] == false){
        visited[current] = true;
        parents[current] = true;

        set<int> neighborhood = get_nonSwitchable_outNeib(graph, current);
        for(auto itr = neighborhood.begin(); itr != neighborhood.end(); itr++){
            if(parents[*itr] == true){
                return true;
            }
            bool result = check_cycle_NS_helper_old(graph, *itr, visited, parents);
            if(result == true && !visited[*itr]){
                return true;
            }
            
        }

        parents[current] = false;
    }

    return false;
}

bool check_cycle_nonSwitchable_old(Graph& graph, int start){
    int graph_size = get<3>(graph);
    vector<bool> visited (graph_size, false);
    vector<bool> parents (graph_size, false);

    return check_cycle_NS_helper_old(graph, start, visited, parents);
}

void build_time_arr(Graph& graph, vector<bool>& visited, vector<int>* state, int current) {
    if(visited[current] == true){
        // revisit
        return;
    }

    // visit
    visited[current] = true;

    set<int> neighbors = get_nonSwitchable_outNeib(graph, current);
    for(auto itr = neighbors.begin(); itr != neighbors.end(); itr++){
        build_time_arr(graph, visited, state, *itr);
    }

    //finish
    (*state).push_back(current);
}

vector<int>* topologicalSort(Graph& graph, vector<int> starts) {
    int graph_size = get<3>(graph);

    vector<int>* sorted_values = new vector<int>;
    //vector<int>& sorted_values = *result;
    vector<bool> visited(graph_size, false);

    int n = starts.size();
    for(int i = 0; i < n; i++){
        build_time_arr(graph, visited, sorted_values, starts[i]);
    }

    reverse((*sorted_values).begin(), (*sorted_values).end());

    return sorted_values;
}

// Slack Example 1.
/*int main() {

    Graph graph = new_graph(26);

    set_type1_edge(graph, 0, 1);
    set_type1_edge(graph, 1, 2);
    set_type1_edge(graph, 2, 3);
    set_type1_edge(graph, 3, 4);
    set_type1_edge(graph, 4, 5);
    set_type1_edge(graph, 5, 6);
    set_type1_edge(graph, 6, 7);

    set_type1_edge(graph, 8, 9);
    set_type1_edge(graph, 9, 10);
    set_type1_edge(graph, 10, 11);
    set_type1_edge(graph, 11, 12);
    set_type1_edge(graph, 12, 13);
    set_type1_edge(graph, 13, 14);

    set_type1_edge(graph, 15, 16);

    set_type1_edge(graph, 17, 18);
    set_type1_edge(graph, 18, 19);
    set_type1_edge(graph, 19, 20);
    set_type1_edge(graph, 20, 21);
    set_type1_edge(graph, 21, 22);
    set_type1_edge(graph, 22, 23);
    set_type1_edge(graph, 23, 24);
    set_type1_edge(graph, 24, 25);

    set_type2_nonSwitchable_edge(graph, 8, 0);
    set_type2_nonSwitchable_edge(graph, 13, 24);

    set_type2_nonSwitchable_edge(graph, 17, 6);
    set_type2_nonSwitchable_edge(graph, 18, 7);
    set_type2_nonSwitchable_edge(graph, 19, 15);
    set_type2_nonSwitchable_edge(graph, 20, 16);
    set_type2_nonSwitchable_edge(graph, 25, 14);

    vector<int> starts;
    starts.push_back(0);
    starts.push_back(8);
    starts.push_back(15);
    starts.push_back(17);
    vector<int> sorted = topologicalSort(graph, starts);
    
    print_graph_concise(graph);
    cout<<"\n"<<endl;
    for(int i = 0; i < (sorted).size(); i++){
        cout<<(sorted)[i]<<" ";
    }
    cout<<endl;

    //cout<<check_cycle_nonSwitchable(graph, 13)<<endl;

    return 0;

}*/

/*int main(){
    Graph graph = new_graph(15);

    set_type1_edge(graph, 0, 1);
    set_type1_edge(graph, 1, 2);
    set_type1_edge(graph, 2, 3);
    set_type1_edge(graph, 4, 5);
    set_type1_edge(graph, 5, 6);
    set_type1_edge(graph, 6, 7);
    set_type1_edge(graph, 7, 8);
    set_type1_edge(graph, 9, 10);
    set_type1_edge(graph, 10, 11);
    set_type1_edge(graph, 11, 12);
    set_type1_edge(graph, 12, 13);
    set_type1_edge(graph, 13, 14);

    set_type2_nonSwitchable_edge(graph, 3, 6);
    set_type2_nonSwitchable_edge(graph, 5, 11);
    set_type2_nonSwitchable_edge(graph, 14, 2);

    cout<<check_cycle_nonSwitchable(graph, 14)<<endl;

    return 0;
}*/

/*int main(){
    Graph graph = new_graph(10);

//     set_type1_edge(graph, 0, 1);
//     set_type1_edge(graph, 1, 2);
//     set_type1_edge(graph, 2, 3);
//     set_type1_edge(graph, 4, 5);
//     set_type1_edge(graph, 5, 6);
//     set_type1_edge(graph, 6, 7);
//     set_type1_edge(graph, 7, 8);
//     set_type1_edge(graph, 8, 9);

//     cout<<check_cycle_nonSwitchable(graph, 0)<<endl;
//     cout<<check_cycle_nonSwitchable(graph, 1)<<endl;
//     cout<<"\n\n"<<endl;

    // Cycle
    // set_type2_nonSwitchable_edge(graph, 1, 6);
    // set_type2_nonSwitchable_edge(graph, 8, 2);
    // cout<<check_cycle_nonSwitchable(graph, 8)<<endl;

    // Not Cycle
    // set_type2_nonSwitchable_edge(graph, 6, 1);
    // set_type2_nonSwitchable_edge(graph, 8, 2);
    // cout<<check_cycle_nonSwitchable(graph, 8)<<endl;

    // Not Cycle
    // set_type2_nonSwitchable_edge(graph, 6, 1);
    // set_type2_nonSwitchable_edge(graph, 2, 8);
    // cout<<check_cycle_nonSwitchable(graph, 2)<<endl;

    // Not Cycle
    // set_type2_nonSwitchable_edge(graph, 1, 6);
    // set_type2_nonSwitchable_edge(graph, 2, 8);
    // cout<<check_cycle_nonSwitchable(graph, 2)<<endl;

    
    
}*/

/*int main() {


    Graph graph = new_graph(5);

    cout<<"graphC1 from graph"<<endl;
    Graph graphC1 = copy_graph(graph);

    set_type1_edge(graphC1, 1, 4);
    set_type2_nonSwitchable_edge(graphC1, 1, 3);
    set_type2_switchable_edge(graphC1, 1, 2);

    print_graph(graph);
    print_graph(graphC1);

    cout<<"graphC2 from graphC1"<<endl;
    Graph graphC2 = copy_graph(graphC1);

    set_type1_edge(graphC1, 3, 0);

    print_graph(graphC1);
    print_graph(graphC2);
    

}*/


/* int main() {


    Graph graph = new_graph(5);

    set_type1_edge(graph, 0, 1);
    set_type1_edge(graph, 1, 2);
    set_type2_nonSwitchable_edge(graph, 0, 3);
    set_type2_nonSwitchable_edge(graph, 3, 1);
    set_type1_edge(graph, 4, 1);
    set_type2_nonSwitchable_edge(graph, 2, 4);

    print_graph(graph);

    cout<<"\n"<<check_cycle_nonSwitchable(graph, 0);

    rem_type2_nonSwitchable_edge(graph, 0, 3);
    cout<<"\n\n"<<check_cycle_nonSwitchable(graph, 0);

    return 0;

}*/ 



// int main(){

//     cout<<"Testing Graph.cpp"<<endl;
  
//     Graph graph = new_graph(5);

//     cout<<"Empty Graph"<<endl;
//     print_graph(graph);

//     // Basic
//     cout<<"Basic"<<endl;
//     set_type1_edge(graph, 1, 2);

//     print_graph(graph);

//     // Set up some type 1 edges
//     cout<<"Set up some type 1 edges"<<endl;
//     set_type1_edge(graph, 2, 4);
//     set_type1_edge(graph, 3, 4);

//     print_graph(graph);

//     // Duplicate insert
//     cout<<"Duplicate insert"<<endl;
//     set_type1_edge(graph, 3, 4);
//     set_type1_edge(graph, 1, 2);

//     print_graph(graph);

//     // Set up some type 2 Non-Switchable Edges
//     cout<<"Set up some type 2 Non-Switchable Edges"<<endl;
//     set_type2_nonSwitchable_edge(graph, 1, 4);
//     set_type2_nonSwitchable_edge(graph, 0, 1);
//     set_type2_nonSwitchable_edge(graph, 2, 0);

//     print_graph(graph);

//     // Set up some type 2 Switchable Edges
//     cout<<"Set up some type 2 Switchable Edges"<<endl;
//     set_type2_switchable_edge(graph, 4, 0);
//     set_type2_switchable_edge(graph, 4, 2);

//     print_graph(graph);
    
//     // Get Out Non-Switchable Edges
//     cout<<"Get Out Non-Switchable Edges"<<endl;
//     set<int> outNeibNS = get_nonSwitchable_outNeib(graph, 2);
//     cout<<"\n"<<endl;
//     for(auto itr = outNeibNS.begin(); itr != outNeibNS.end(); itr++){
//         cout<<*itr<<" ";
//     }
//     cout<<"\n"<<endl;

//     // Get In Non-Switchable Edges
//     cout<<"Get In Non-Switchable Edges"<<endl;
//     set<int> inNeibNS = get_nonSwitchable_inNeib(graph, 4);
//     cout<<"\n"<<endl;
//     for(auto itr = inNeibNS.begin(); itr != inNeibNS.end(); itr++){
//         cout<<*itr<<" ";
//     }
//     cout<<"\n"<<endl;

//     // Get copy of Out Switchable Edges
//     cout<<"Get Out Switchable Edges"<<endl;
//     set<int> outNeibSC = get_switchable_outNeib(graph, 4);
//     cout<<"\n"<<endl;
//     for(auto itr = outNeibSC.begin(); itr != outNeibSC.end(); itr++){
//         cout<<*itr<<" ";
//     }
//     cout<<"\n"<<endl;

//     // Get copy of In Switchable Edges
//     cout<<"Get In Switchable Edges"<<endl;
//     set<int> inNeibSC = get_switchable_inNeib(graph, 2);
//     cout<<"\n"<<endl;
//     for(auto itr = inNeibSC.begin(); itr != inNeibSC.end(); itr++){
//         cout<<*itr<<" ";
//     }
//     cout<<"\n"<<endl;

//     // Check copy status
//     auto itr1 = outNeibSC.find(0);
//     auto itr2 = inNeibSC.find(4);
//     outNeibSC.erase(itr1);
//     inNeibSC.erase(itr2);

//     print_graph(graph);

//     // Get no-copy of Out Switchable Edges
//     cout<<"Get Out Switchable Edges"<<endl;
//     set<int>& outNeibS = get_switchable_outNeib(graph, 4);
//     cout<<"\n"<<endl;
//     for(auto itr = outNeibS.begin(); itr != outNeibS.end(); itr++){
//         cout<<*itr<<" ";
//     }
//     cout<<"\n"<<endl;

//     // Get no-copy of In Switchable Edges
//     cout<<"Get In Switchable Edges"<<endl;
//     set<int>& inNeibS = get_switchable_inNeib(graph, 2);
//     cout<<"\n"<<endl;
//     for(auto itr = inNeibS.begin(); itr != inNeibS.end(); itr++){
//         cout<<*itr<<" ";
//     }
//     cout<<"\n"<<endl;

//     // Uncomment to test that memory is actually aliased
//     /*itr1 = outNeibS.find(0);
//     itr2 = inNeibS.find(4);
//     outNeibS.erase(itr1);
//     inNeibS.erase(itr2);

//     print_graph(graph);
//     */

//     // Get Out Edges
//     /*cout<<"Get Out Edges"<<endl;
//     set<int> outNeib = get_outNeighbors(graph, 4);
//     for(auto itr = outNeib.begin(); itr != outNeib.end(); itr++){
//         cout<<*itr<<" ";
//     }
//     cout<<"\n"<<endl;

//     // Get In Edges
//     cout<<"Get In Edges"<<endl;
//     set<int> inNeib = get_inNeighbors(graph, 4);
//     for(auto itr = inNeib.begin(); itr != inNeib.end(); itr++){
//         cout<<*itr<<" ";
//     }
//     cout<<"\n"<<endl;

//     // Remove type 1 edges
//     cout<<"Remove type 1 edges"<<endl;
//     rem_type1_edge(graph, 3, 4);

//     print_graph(graph);

//     // Remove type 2 Non-Switchable Edges
//     cout<<"Remove type 2 Non-Switchable Edges"<<endl;
//     rem_type2_nonSwitchable_edge(graph, 0, 1);

//     print_graph(graph);

//     // Remove type 2 Switchable Edges
//     cout<<"Remove type 2 Switchable Edges"<<endl;
//     rem_type2_switchable_edge(graph, 4, 2);

//     print_graph(graph);
// */
// //    return 0;

// }


