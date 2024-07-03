//
// Created by Zhe Chen on 5/8/21.
//
#pragma once

#ifndef CBS_K_PATH_H
#define CBS_K_PATH_H

#include "Conflict.h"
#include "common.h"

struct PathEntry
{
    Location Loc;
    list<Location> occupations;
    vector<bool> singles;
    int heading;
    bool single;
    int actionToHere;
    int timeStep;
    int self_conflict;
    PathEntry():Loc(-1,-1) {}
    PathEntry(Location loc):Loc(loc) { }
    bool operator== (const PathEntry& other) const
    {
        return (this->Loc == other.Loc);
    }
    std::list<Location> locations; // all possible locations at the same time step
    std::list<Conflict> conflist;
};

typedef std::vector<PathEntry> Path;


#endif //CBS_K_PATH_H
