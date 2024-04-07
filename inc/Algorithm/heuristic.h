#pragma once
#include "types.h"

// NOTE(rivers): we almost copy codes from https://github.com/Jiaoyang-Li/CBSH2-RTC/blob/main/src/CBSHeuristic.cpp
enum HeuristicType {
    ZERO,
    CG_GREEDY,
    WCG_GREEDY
};

class HeuristicManager {
public:
    HeuristicType type=HeuristicType::ZERO;

    HeuristicManager(HeuristicType type);

    double computeInformedHeuristics(ADG & adg, vector<int> & tp_ordered_states, vector<int> & tp_times, double time_limit);
    void buildCardinalConflictGraph(ADG & adg, vector<int> & tp_ordered_states, vector<int> & tp_times, vector<int> & CG, bool weighted);
    double greedyMatching(const std::vector<int> & CG, int num_vertices);

};