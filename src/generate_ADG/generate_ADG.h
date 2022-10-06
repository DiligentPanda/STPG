#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "../graph/graph.h"

using namespace std;

typedef tuple<int, int> Location;
typedef vector<tuple<int, int>> Path;
typedef vector<Path> Paths;

Graph construct_ADG(char* fileName);
