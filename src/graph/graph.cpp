#include <stack>
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

Graph copy_graph(Graph& graph){

    // removed until verification of tuple implementation

    return graph;


}

void free_graph(Graph& graph){
    delete[] get<0>(graph).first;
    delete[] get<0>(graph).second;

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


bool check_cycle_NS_helper(Graph& graph, int current, vector<bool>& visited, vector<bool>& parents){
    /*cout<<current<<endl;
    for(int i = 0; i < parents.size(); i++){
        if(parents[i]){
            cout<<i<<" ";
        }
    }
    cout<<"\n\n"<<endl;*/
    if(visited[current] == false){
        visited[current] = true;
        parents[current] = true;

        set<int> neighborhood = get_nonSwitchable_outNeib(graph, current);
        for(auto itr = neighborhood.begin(); itr != neighborhood.end(); itr++){
            if(parents[*itr] == true){
                return true;
            }
            bool result = check_cycle_NS_helper(graph, *itr, visited, parents);
            if(result == true){
                return true;
            }
            
        }

        parents[current] = false;
    }

    return false;
}

bool check_cycle_nonSwitchable(Graph& graph, int start){
    int graph_size = get<3>(graph);
    vector<bool> visited (graph_size, false);
    vector<bool> parents (graph_size, false);

    return check_cycle_NS_helper(graph, start, visited, parents);
}





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



