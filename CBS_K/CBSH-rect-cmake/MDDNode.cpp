//
// Created by Zhe Chen on 24/3/20.
//
#include "MDDNode.h"


MDDNode::MDDNode(std::list<Location> currlocs, MDDNode* parent, bool train)
{
    locs = currlocs;
    this->parent = parent;
    this->train = train;
    if(parent == NULL)
        level = 0;
    else
    {
        level = parent->level + 1;
        parents.push_back(parent);
    }
}
