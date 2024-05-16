#pragma once
#include <queue>
#include "Algorithm/SearchNode.h"
#include <cstdlib>
#include <set>
#include <iostream>
#include "define.h"

class OpenList {
public:
    OpenList(COST_TYPE w_focal=1): w_focal(w_focal), focal(w_focal>1), f_lowerbound(0.0) {
        if (focal) {
            std::cout<<"focal search with w="<<w_focal<<std::endl;
        } else {
            std::cout<<"normal A* search"<<std::endl;
        }

    }

    COST_TYPE w_focal;
    bool focal;
    COST_TYPE f_lowerbound;

    // used for A*
    priority_queue<shared_ptr<SearchNode>, vector<shared_ptr<SearchNode> >, SearchNode::CompareOpenHeap > open_heap;

    // used for focal search, we can also think about boost::MultiIndex in the future.
    // NOTE: we need to use multiset instead of set because f-values can be the same and the inequality of keys are determined automatically
    // by the less operator of the key type.
    std::multiset<shared_ptr<SearchNode>, SearchNode::CompareOpenSet >  open_set;
    boost::heap::pairing_heap<shared_ptr<SearchNode>, boost::heap::compare<SearchNode::CompareFocal> >  focal_list;

    inline void push(const shared_ptr<SearchNode> & node) {
        if (!focal) {
            open_heap.push(node);
        } else {
            node->open_handle=open_set.insert(node);
            if (node->f <= f_lowerbound * w_focal) {
                focal_list.push(node);
            }
        }
    }

    inline const shared_ptr<SearchNode> & top() {
        if (!focal) {
            return open_heap.top();
        } else {
            _update_focal_list();

            return focal_list.top();
        }
    }
    
    inline const shared_ptr<SearchNode> pop() {
        if (!focal) {
            auto node = open_heap.top();
            open_heap.pop();
            return node;
        } else {
            // update focal list before poping if the lowerbound is creased
            _update_focal_list();

            // pop from focal
            auto focal_top = focal_list.top();
            focal_list.pop();
            open_set.erase(focal_top->open_handle);
            
            return focal_top;
        }
    }

    size_t size() const {
        if (!focal) {
            return open_heap.size();
        } else {
            return open_set.size();
        }
    }


private:
    void _update_focal_list() {
        auto open_top=*open_set.cbegin();
        if (open_top->f > f_lowerbound) {
            COST_TYPE old_threshold = f_lowerbound*w_focal;
            f_lowerbound = std::max(open_top->f, f_lowerbound);
            COST_TYPE new_threshold = f_lowerbound*w_focal;

            // begin_key
            auto begin_key=std::make_shared<SearchNode>(old_threshold);
            auto end_key=std::make_shared<SearchNode>(new_threshold);

            auto begin_iter=open_set.upper_bound(begin_key);
            auto end_iter=open_set.upper_bound(end_key);

            for (auto it=begin_iter; it!=end_iter; ++it) {
                focal_list.push(*it);
            }
        }
    }

};