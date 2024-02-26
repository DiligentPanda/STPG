#ifndef TYPES
#define TYPES

#include <stdlib.h>
#include <tuple>
#include <vector>
#include <set>
#include <utility>

using namespace std;

// <arr[time]=vertex, arr[vertex]=time>
typedef pair<vector<int>*, vector<int>*> sortResult;

// <outNeighbors, inNeighbors>
// set<int> * is an array of size = total state number
typedef pair<set<int>*, set<int>*> subGraph;

// <type1 Graph, non-switchable Type2 Graph, switchable Type2 Graph, total num of nodes/states>
typedef tuple<subGraph, subGraph, subGraph, int> Graph;
// <coordinates>
typedef pair<int, int> Location;
// <location, timestep>
typedef vector<pair<Location, int>> Path;
// paths for all agents
typedef vector<Path> Paths;
// <adg graph, agent paths, accumulated state counts from agent 0 to agent k, k=0 to n-1>
// accumulated state counts are used for computing the global index for a state.
typedef tuple<Graph, Paths, vector<int>> ADG;

// used in graph-based search
typedef tuple<ADG, int, vector<int>*> Node;

// used in execution-based search
typedef tuple<ADG, int, int, vector<int>> slow_Node;


#endif

