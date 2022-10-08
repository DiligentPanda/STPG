#ifndef   TYPES
#define   TYPES

#include <stdlib.h>
#include <tuple>
#include <vector>
#include <set>
#include <utility>

using namespace std;

enum edgeType{
    type1,
    type2,
    nullEdge
};

// <outNeighbors, inNeighbors>
typedef tuple<set<edgeType>*, set<edgeType>*> subGraph;

// <type1 Graph, non-switchable Type2 Graph, switchable Type2 Graph, num nodes>
typedef tuple<subGraph, subGraph, subGraph, int> Graph;
typedef pair<int, int> Location;
typedef vector<Location> Path;
typedef vector<Path> Paths;
typedef tuple<Graph, Paths, vector<int>> ADG;

#endif