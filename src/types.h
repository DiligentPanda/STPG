#ifndef   TYPES
#define   TYPES

#include <stdlib.h>
#include <tuple>
#include <vector>
#include <utility>

using namespace std;

typedef int edgeType;

typedef tuple<edgeType**, int, int> Graph;
typedef pair<int, int> Location;
typedef vector<Location> Path;
typedef vector<Path> Paths;
typedef tuple<Graph, Paths, vector<int>> ADG;

#endif