#include <tuple>
#include <vector>
#include <utility>

using namespace std;

typedef int edgeType;
const edgeType NULL_EDGE = 0;
const edgeType TYPE1_EDGE = 1;
const edgeType TYPE2_EDGE = 2;

typedef tuple<edgeType**, int, int> Graph;
typedef pair<int, int> Location;
typedef vector<Location> Path;
typedef vector<Path> Paths;
typedef pair<Graph, Paths, vector<int>> ADG;