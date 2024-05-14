// #include <iostream>
// #include <vector>
// #include <algorithm>
// #include <queue>
// #include <unordered_map>
// #include <memory>

// #include "graph/graph.h"
// #include "ADG/generate_ADG.h"
// #include "graph/graph.h"
// #include <stack>

// bool detect_cycle(Graph & graph, std::vector<bool> & visited, std::vector<bool> & recursion_stack,std::vector<int> & parent, int curr, std::pair<int,int> & edge_to_reverse) {
//     visited[curr]=true;
//     recursion_stack[curr]=true;

//     // check child
//     set<int> && out_neighbors=get_outNeighbors(graph, curr);
//     for (auto & next: out_neighbors) {
//         if (visited[next]) {
//             if (recursion_stack[next]) {
//                 // find a cycle
//                 parent[next]=curr;

//                 // trace back parent and find a switchable edge
//                 int p=curr;
//                 int c=next;           
//                 do {
//                     if (get_type2_switchable_edge(graph, p, c)) {
//                         edge_to_reverse=std::make_pair(p,c);
//                         break;
//                     }
//                     c=p;
//                     p=parent[c];
//                 } while (p!=curr);
//                 return true;
//             }
//         } else {
//             parent[next]=curr;
//             if (detect_cycle(graph, visited, recursion_stack, parent, next, edge_to_reverse)) {
//                 // find a cycle
//                 return true;
//             }
//         }
//     }

//     recursion_stack[curr]=false;
//     return false;
// }

// bool detect_cycle(Graph & graph, std::pair<int,int> & edge_to_reverse) {
//     int size=get<3>(graph);

//     std::vector<bool> visited(size,false);
//     std::vector<bool> recursion_stack(size,false);
//     std::vector<int> parent(size,-1);

//     for (int i=0;i<size;++i) {
//         if (!visited[i]) {
//             bool has_cycle=detect_cycle(graph,visited,recursion_stack,parent,i,edge_to_reverse);
//             // std::cout<<"cd "<<i<<" "<<has_cycle<<std::endl;
//             if (has_cycle) {
//                 return true;
//             }
//         }
//     }

//     return false;
// }

// // without checking
// void reverse_edge(Graph & graph, int s1, int s2) {
//     int back1=s2+1;
//     int back2=s1-1;
//     rem_type2_switchable_edge(graph, s1, s2);
//     set_type2_nonSwitchable_edge(graph, back1, back2);
//     // std::cout<<"reverse: "<<s1<<"->"<<s2<<" => "<<back1<<"->"<<back2<<std::endl;
// }


// bool check_grouping(std::vector<int> &a1,std::vector<int>& a2) {
//     // current we only support the simplest situations, both a1 and a2 have n distinct vertices 1...n but with different orders.
//     int num1=a1.size();
//     int num2=a2.size();

//     if (num1!=num2) {
//         std::cout<<"not supported now"<<std::endl;
//     }

//     // build graph
//     // for n=3, (4) and (5) are extra state/vertices
//     // -1 1 2 3 (4)
//     //  0        2 1 3 (5)

//     auto paths_ptr=make_shared<Paths>(2);
//     auto & paths=*paths_ptr;

//     Location l(-1,0);
//     paths[0].emplace_back(l,0);
//     for (int i=0;i<num1;++i) {
//         Location l(a1[i],0);
//         int t=i+1;
//         paths[0].emplace_back(l,t);
//     }
//     l=Location(num1+num2,0);
//     paths[0].emplace_back(l,num1+1);

//     l=Location(0,0);
//     paths[0].emplace_back(l,0);
//     for (int i=0;i<num2;++i) {
//         Location l(a2[i],0);
//         int t=i+num1+1; // ensure in the type 2 edge, a1 points to a2
//         paths[1].emplace_back(l,t);
//     }
//     l=Location(num1+num2+1,0);
//     paths[1].emplace_back(l,num1+num2+1);

//     ADG adg = construct_ADG(paths_ptr, false);

//     // for each edge
//     bool grouped=true;
//     for (int i=1;i<=num1;++i) {
//         ADG _adg = copy_ADG(adg);
//         auto & graph = get<0>(_adg);
//         int state_a1 = i+1;
//         set<int> & out_neighbors = get_switchable_outNeib(graph, state_a1);
//         // std::cout<<"sz: "<<out_neighbors.size()<<std::endl;
//         int state_a2 = *out_neighbors.begin();
//         reverse_edge(graph,state_a1,state_a2);
//         int cnt=1;
//         for (;cnt<=num1;++cnt) {
//             // return a switchable type 2 edge
//             std::pair<int,int> edge_to_reverse(-1,-1); 
//             bool has_cycle=detect_cycle(graph, edge_to_reverse);
//             // std::cout<<cnt<<" "<<has_cycle<<" "<<num1<<std::endl;
//             // if no cycle
//             if (!has_cycle) {
//                 break;
//             } else {
//                 if (edge_to_reverse.first<0) {
//                     std::cout<<"logical error"<<std::endl;
//                 } else {
//                     reverse_edge(graph,edge_to_reverse.first,edge_to_reverse.second);
//                 }
//             }
//         }
//         // std::cout<<i<<" "<<cnt<<std::endl;
//         if (cnt!=num1) {
//             grouped=false;
//             break;
//         }
//     }

//     return grouped;
// }

// void enumerate_grouping(int num) {
//     // we need to perumate 

//     std::vector<int> a1(num);
//     for (int i=1;i<=num;++i) {
//         a1[i-1]=i;
//     }

//     auto a2=a1;

//     do {
//         for (int i=0;i<num;++i) {
//             std::cout<<a2[i]<<" ";
//         }
//         std::cout<<std::endl;
//         bool succ=check_grouping(a1,a2);
//         std::cout<<"can be grouped: "<<succ<<std::endl;
//     } while (std::next_permutation(a2.begin(),a2.end()));

// }


// int main() {
    
//     int num_start=2;
//     int num_end=8;


//     for (auto num=num_start;num<=num_end;++num) {
//         enumerate_grouping(num);
//     }

//     return 0;
// }