#pragma once
#include <unordered_map>
#include "util/util.h"
#include "define.h"

/*
This might not be a good idea, but fine.
*/

struct Edge {
    COST_TYPE cost;
};


class EdgeManager {
public:
    EdgeManager() {}
    std::unordered_map<std::pair<int,int>, Edge, PairIntHash> edges;
    
    inline void add_edge(int a, int b, COST_TYPE cost) {
        edges[std::make_pair(a,b)] = {cost};
    }

    inline Edge & get_edge(int a, int b) {
        return edges.at(std::make_pair(a,b));
    }

};