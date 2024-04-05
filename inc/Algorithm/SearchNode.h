#pragma once
#include "types.h"

struct SearchNode {
    SearchNode(
        const ADG & adg, 
        const int g, 
        const int h, 
        const shared_ptr<vector<int> > & longest_path_lengths):
        adg(adg), 
        g(g), 
        h(h), 
        longest_path_lengths(longest_path_lengths), 
        f(g+h)
        {

    }

    ADG adg;
    int g;
    int h;
    shared_ptr<vector<int> > longest_path_lengths;
    int f;

    struct Compare {
      public:
        bool operator() (const shared_ptr<SearchNode> & s1, const shared_ptr<SearchNode> & s2)
        {
            return s1->f > s2->f;
        }
    };

};