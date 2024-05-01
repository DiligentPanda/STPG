#pragma once
#include "types.h"
#include <boost/heap/pairing_heap.hpp>

struct SearchNode {
    SearchNode(
        const shared_ptr<Graph> & adg, 
        const double g, 
        const double h, 
        const shared_ptr<vector<int> > & _longest_path_lengths,
        const shared_ptr<vector<shared_ptr<map<int,int> > > > & _reverse_longest_path_lengths,
        int num_sw,
        const shared_ptr<SearchNode> & _parent
        ):
        adg(adg), 
        g(g), 
        h(h), 
        longest_path_lengths(_longest_path_lengths), 
        reverse_longest_path_lengths(_reverse_longest_path_lengths),
        f(g+h),
        num_sw(num_sw),
        parent(_parent)
        {

    }

    SearchNode(double f): f(f) {};

    shared_ptr<Graph> adg;
    double g;
    double h;
    shared_ptr<vector<int> > longest_path_lengths;
    shared_ptr<vector<shared_ptr<map<int,int> > > > reverse_longest_path_lengths;
    double f;
    int num_sw; // the number of switchable (edge groups)

    shared_ptr<SearchNode> parent;

    struct CompareOpenSet {
        public:
            bool operator() (const shared_ptr<SearchNode> & s1, const shared_ptr<SearchNode> & s2) const
            {
                return s1->f < s2->f;
            }
    };

    struct CompareOpenHeap {
        public:
            bool operator() (const shared_ptr<SearchNode> & s1, const shared_ptr<SearchNode> & s2) const
            {
                return s1->f > s2->f;
            }
    };

    struct CompareFocal {
        public:
            bool operator() (const shared_ptr<SearchNode> & s1, const shared_ptr<SearchNode> & s2) const
            {
                if (s1->num_sw != s2->num_sw)
                    return s1->num_sw > s2->num_sw;
                else if (s1->h != s2->h)
                    return s1->h > s2->h;
                else 
                    return s1->f > s2->f;
            }
    };

    std::multiset<shared_ptr<SearchNode>, SearchNode::CompareOpenSet>::iterator open_handle;
    // boost::heap::pairing_heap<shared_ptr<SearchNode>, boost::heap::compare<CompareOpen> >::handle_type open_handle;
    // boost::heap::pairing_heap<shared_ptr<SearchNode>, boost::heap::compare<CompareFocal> >::handle_type focal_handle;
    
};