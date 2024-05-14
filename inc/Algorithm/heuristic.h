#pragma once
#include "graph/graph.h"
#include "define.h"

// NOTE(rivers): we almost copy codes from https://github.com/Jiaoyang-Li/CBSH2-RTC/blob/main/src/CBSHeuristic.cpp
enum HeuristicType {
    ZERO,
    CG_GREEDY,
    WCG_GREEDY,
    FAST_WCG_GREEDY
};

class HeuristicManager {
public:
    HeuristicType type=HeuristicType::ZERO;

    HeuristicManager(HeuristicType type);

    COST_TYPE computeInformedHeuristics(
        const shared_ptr<Graph> & graph, 
        const shared_ptr<vector<COST_TYPE> > & longest_paths, 
        const shared_ptr<vector<shared_ptr<map<int,COST_TYPE> > > > & reverse_longest_paths,
        COST_TYPE time_limit,
        bool fast_approximate
    );
    void buildCardinalConflictGraph(
        const shared_ptr<Graph> & graph, 
        const shared_ptr<vector<COST_TYPE> > & longest_paths, 
        const shared_ptr<vector<shared_ptr<map<int,COST_TYPE> > > > & old_reverse_longest_path_lengths_ptr, 
        vector<COST_TYPE> & CG, 
        bool weighted,
        bool fast_approximate
    );
    COST_TYPE greedyMatching(const std::vector<COST_TYPE> & CG, int num_vertices);

};