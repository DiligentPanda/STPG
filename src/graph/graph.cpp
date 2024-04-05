#include <stack>
#include <algorithm>
#include "graph/graph.h"
#include <cstdlib>

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
    auto type1GInNeighbors = make_shared<vector<set<int> > >(n);
    auto type1OutNeighbors = make_shared<vector<set<int> > >(n);
    subGraph type1G = make_pair(type1OutNeighbors,type1GInNeighbors);

    auto type2NonSwitchableGInNeighbors = make_shared<vector<set<int> > >(n);
    auto type2NonSwitchableGOutNeighbors = make_shared<vector<set<int> > >(n);
    subGraph type2NonSwitchableG = make_pair(type2NonSwitchableGOutNeighbors, type2NonSwitchableGInNeighbors);

    auto type2SwitchableGInNeighbors = make_shared<vector<set<int> > >(n);
    auto type2SwitchableGOutNeighbors = make_shared<vector<set<int> > >(n);
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
    (*type1G.first)[n1].insert(n2);
    (*type1G.second)[n2].insert(n1);

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
    (*type2NSG.first)[n1].insert(n2);
    (*type2NSG.second)[n2].insert(n1);

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
    (*type2SG.first)[n1].insert(n2);
    (*type2SG.second)[n2].insert(n1);

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
    auto itr = (*type1G.first)[n1].find(n2);
    if(itr != (*type1G.first)[n1].end()){
        (*type1G.first)[n1].erase(itr);
    }

    itr = (*type1G.second)[n2].find(n1);
    if(itr != (*type1G.second)[n2].end()){
        (*type1G.second)[n2].erase(itr);
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
    auto itr = (*type2NSG.first)[n1].find(n2);
    if(itr != (*type2NSG.first)[n1].end()){
        (*type2NSG.first)[n1].erase(itr);
    }

    itr = (*type2NSG.second)[n2].find(n1);
    if(itr != (*type2NSG.second)[n2].end()){
        (*type2NSG.second)[n2].erase(itr);
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
    (*type2NSG.first)[n].clear();  

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
    auto itr = (*type2SG.first)[n1].find(n2);
    if(itr != (*type2SG.first)[n1].end()){
        (*type2SG.first)[n1].erase(itr);
    }

    itr = (*type2SG.second)[n2].find(n1);
    if(itr != (*type2SG.second)[n2].end()){
        (*type2SG.second)[n2].erase(itr);
    }

    return;
    
}

bool get_type1_edge(Graph& graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type1G = get<0>(graph);
    auto itr = (*type1G.first)[n1].find(n2);
    bool result = itr == (*type1G.first)[n1].end() ? false : true;

    return result;
}

bool get_type2_nonSwitchable_edge(Graph& graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type2NSG = get<1>(graph);
    auto itr = (*type2NSG.first)[n1].find(n2);
    bool result = itr == (*type2NSG.first)[n1].end() ? false : true;

    return result;
}

bool get_type2_switchable_edge(Graph& graph, int n1, int n2){
    if(error_check_node_range(graph, n1, n2) == false){
        return false;
    }

    subGraph& type2SG = get<2>(graph);
    auto itr = (*type2SG.first)[n1].find(n2);
    bool result = itr == (*type2SG.first)[n1].end() ? false : true;

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
    result.insert((*type1G.first)[n].begin(), (*type1G.first)[n].end());
    result.insert((*type2NSG.first)[n].begin(), (*type2NSG.first)[n].end());

    return result;

}

set<int> get_nonSwitchable_inNeib(Graph& graph, int n){
    subGraph& type1G = get<0>(graph);
    subGraph& type2NSG = get<1>(graph);
    set<int> result;
    result.insert((*type1G.second)[n].begin(), (*type1G.second)[n].end());
    result.insert((*type2NSG.second)[n].begin(), (*type2NSG.second)[n].end());

    return result;
}

set<int>& get_switchable_outNeib(Graph& graph, int n){
    subGraph& type2SG = get<2>(graph);
    return (*type2SG.first)[n];
}

set<int>& get_switchable_inNeib(Graph& graph, int n){
    subGraph& type2SG = get<2>(graph);
    return (*type2SG.second)[n];
}

set<int>& get_type2_nonSwitchable_inNeib(Graph& graph, int n) {
    return (*get<1>(graph).second)[n];
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
        for(auto itr = (*graph2S.first)[i].begin(); itr != (*graph2S.first)[i].end(); itr++){
            (*graph2NS.first)[i].insert(*itr);
        }
        (*graph2S.first)[i].clear();
        for(auto itr = (*graph2S.second)[i].begin(); itr != (*graph2S.second)[i].end(); itr++){
            (*graph2NS.second)[i].insert(*itr);
        }
        (*graph2S.second)[i].clear();
    }

    return;
}

void reverse_nonSwitchable_edges_basedOn_LongestPathValues(Graph& graph, shared_ptr<vector<int> > & values) {
    int graph_size = get<3>(graph);
    subGraph& graph2S = get<2>(graph);
    auto & times=*values;

    std::vector<std::pair<int,int> > need_to_reverse;
    for (int i=0;i<graph_size;++i) {
        // find all out neighbors
        for (auto itr=(*graph2S.first)[i].begin(); itr!=(*graph2S.first)[i].end();++itr) {
            int j=*itr;

            int time_i=times[i];
            int time_j=times[j];

            int back_i=j+1;
            int back_j=i-1;
            int time_back_i=times[back_i];
            int time_back_j=times[back_j];

            if (time_i>=time_j) {
                if (time_back_i>=time_back_j) {
                    std::cout<<"error in set_switchable_nonSwitchable_basedOn_LongestPath: conflict not solved yet!"<<std::endl;
                    std::cout<<"time_i="<<time_i<<" time_j="<<time_j<<std::endl;
                    std::cout<<"time_back_i="<<time_back_i<<" time_back_j="<<time_back_j<<std::endl;
                    exit(200);
                }
                // need to reverse
                need_to_reverse.emplace_back(i,j);
            }
        }
    }

    fix_switchable_edges(graph, need_to_reverse, true);

    return;
}


subGraph copy_subgraph(subGraph & subgraph) {
    int n=(*subgraph.first).size();
    auto type2SwitchableGInNeighbors = make_shared<vector<set<int> > >(n);
    for(int i = 0; i < n; i++){
        copy((*subgraph.second)[i].begin(), (*subgraph.second)[i].end(), inserter((*type2SwitchableGInNeighbors)[i], (*type2SwitchableGInNeighbors)[i].begin()));
    }
    auto type2SwitchableGOutNeighbors = make_shared<vector<set<int> > >(n);
    for(int i = 0; i < n; i++){
        copy((*subgraph.first)[i].begin(), (*subgraph.first)[i].end(), inserter((*type2SwitchableGOutNeighbors)[i], (*type2SwitchableGOutNeighbors)[i].begin()));
    }

    return {type2SwitchableGOutNeighbors, type2SwitchableGInNeighbors};
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

    subGraph type2NonSwitchableG = copy_subgraph(get<1>(graph));
    subGraph type2SwitchableG = copy_subgraph(get<2>(graph));
   
    Graph graphC = make_tuple(type1G, type2NonSwitchableG, type2SwitchableG, n);
    return graphC;

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
        auto g = (*type1G.first)[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = (*type1G.second)[i];
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
        auto g = (*type2NSG.first)[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = (*type2NSG.second)[i];
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
        auto g = (*type2SG.first)[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = (*type2SG.second)[i];
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
        auto g = (*type1G.first)[i];
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
        auto g = (*type1G.second)[i];
        if(g.size() == 0){
            continue;
        }
        cout<<"1IN: "<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
    cout<<endl;

    cout<<"Type 2 Non-Switchable Graph\n"<<endl;

    cout<<"Out Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = (*type2NSG.first)[i];
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
        auto g = (*type2NSG.second)[i];
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
        auto g = (*type2SG.first)[i];
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
        auto g = (*type2SG.second)[i];
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
        auto g = (*type2SG.first)[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = (*type2SG.second)[i];
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
        auto g = (*type2NSG.first)[i];
        cout<<i<<": ";
        for(auto itr = g.begin(); itr != g.end(); itr++){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }

    cout<<"In Neighbors"<<endl;
    for(int i = 0; i < size; i++){
        auto g = (*type2NSG.second)[i];
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


    set<int> neighborhood1 = (*get<0>(graph).first)[current];
    set<int> neighborhood2NS = (*get<1>(graph).first)[current];
    
    if (type2_flag){
        set<int> reversible_edge = (*get<0>(graph).second)[current];
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

bool isCyclicUtil(Graph &graph, int v, vector<bool> &visited,
                         vector<bool> &recStack)
{
    if (visited[v] == false) {
        visited[v] = true;
        recStack[v] = true;
 
        set<int> neighbors = get_nonSwitchable_outNeib(graph, v);
        for (auto i = neighbors.begin(); i != neighbors.end(); i++){
            if (!visited[*i]
                && isCyclicUtil(graph, *i, visited, recStack))
                return true;
            else if (recStack[*i])
                return true;
        }
    }
 
    recStack[v] = false;
    return false;
}

bool check_cycle_dfs(Graph& graph, int start) {
    int graph_size = get<3>(graph);
    vector<bool> visited (graph_size, false);
    vector<bool> recStack (graph_size, false);

    return isCyclicUtil(graph, start, visited, recStack);
}

bool check_cycle_dfs(Graph& graph, std::vector<int>& starts) {
  int graph_size = get<3>(graph);
  vector<bool> visited (graph_size, false);
  vector<bool> recStack (graph_size, false);
  
  for (auto start: starts) {
    if (!visited[start]) {
      if (isCyclicUtil(graph, start, visited, recStack)) {
        return true;
      }
    }
  }
  return false;
}

void build_time_arr(Graph& graph, vector<bool>& visited, shared_ptr<vector<int> > sorted_vertecies, shared_ptr<vector<int> > sorted_times, int current, int& time) {
    if(visited[current] == true){
        // revisit
        return;
    }

    // visit
    visited[current] = true;

    // assign time to all successors first
    set<int> neighbors = get_nonSwitchable_outNeib(graph, current);
    for(auto itr = neighbors.begin(); itr != neighbors.end(); itr++){
        build_time_arr(graph, visited, sorted_vertecies, sorted_times, *itr, time);
    }

    //finish
    // (*state).push_back(current);
    (*sorted_vertecies)[time] = current;
    (*sorted_times)[current] = time;
    time--;
}

/* (incremental?) topological sort

COMMENT(rivers): this is a recursive implementation, which might not be necessary. We can just count the in-/out-degree.
TODO(rivers): it seems that the incremental version has not been implemented yet.

input:
    sortResult: (init) sorted restuls, including the following two arrays of the same size.
        topological order -> vertex global idx
        vertex global idx -> topological order
    agent_starts: current agent vertex local idxs
    u: ?
    v: ?
return:
    sortResult: see input sortResult
*/ 
sortResult topologicalSort(Graph& graph, sortResult state, const vector<int> & starts, int u, int v) {
    int graph_size = get<3>(graph);
  
    shared_ptr<vector<int> > time_arr = state.first;
    shared_ptr<vector<int> > vertex_arr = state.second;

    if(time_arr == nullptr && vertex_arr == nullptr) {
        auto sorted_vertecies = make_shared<vector<int> > (graph_size, -1);
        auto sorted_times = make_shared<vector<int> > (graph_size, -1);
        vector<bool> visited(graph_size, false);
        int time = graph_size - 1;

        int n = starts.size();
        for(int i = 0; i < n; i++){
            build_time_arr(graph, visited, sorted_vertecies, sorted_times, starts[i], time);
        }

        sortResult ret_val = make_pair(sorted_vertecies, sorted_times);
        return ret_val;
    }
    else{
        // Error scenario
        sortResult ret_val = make_pair(nullptr, nullptr);
        return ret_val;
    }

    
}

std::pair<int,int> fix_switchable_edge(Graph & graph, int out_state_idx, int in_state_idx, bool reverse) {
    rem_type2_switchable_edge(graph, out_state_idx, in_state_idx);
    if (!reverse){
      set_type2_nonSwitchable_edge(graph, out_state_idx, in_state_idx);
      return {out_state_idx, in_state_idx};
    } else {
      int back_out_state_idx=in_state_idx+1;
      int back_in_state_idx=out_state_idx-1;
      set_type2_nonSwitchable_edge(graph, back_out_state_idx, back_in_state_idx);
      return {back_out_state_idx, back_in_state_idx};
    }
}

std::vector<std::pair<int,int> > fix_switchable_edges(Graph & graph, std::vector<std::pair<int,int> > & edges, bool reverse) {
  std::vector<std::pair<int,int> > fixed_edges;

  for (auto & edge:edges) {
    fixed_edges.emplace_back(fix_switchable_edge(graph,edge.first,edge.second,reverse));
  }

  return fixed_edges;
}