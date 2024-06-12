#pragma once
#include <tuple>
#include <list>
#include <vector>
#include <set>
#include <ctime>
#include <fstream>
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <chrono>
#include <utility>
#include <random>
#include <map>
#include <boost/heap/pairing_heap.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

using boost::heap::pairing_heap;
using boost::heap::compare;
using boost::unordered_map;
using boost::unordered_set;
using std::vector;
using std::list;
using std::set;
using std::map;
using std::get;
using std::tuple;
using std::make_tuple;
using std::pair;
using std::make_pair;
using std::tie;
using std::min;
using std::max;
using std::shared_ptr;
using std::make_shared;
using std::clock;
using std::cout;
using std::endl;
using std::ofstream;
using std::cerr;
using std::string;
using std::hash;
using namespace std::chrono;
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<float> fsec;

#define MAX_TIMESTEP INT_MAX / 2
#define MAX_COST INT_MAX / 2
#define MAX_NODES INT_MAX / 2

namespace ConstrainedGraph {
    
class Location
{
public:
    int location;
    int index;
    Location(int loc = -1):location(loc), index(-1){}
    // Location(const Location& Loc): location(Loc.location), index(Loc.index){}
    Location(int loc, int idx): location(loc), index(idx){}

    bool operator== (const Location& other) const
    {
        return ((this->location == other.location) &&
                (this->index == other.index));
    }
};

struct PathEntry
{
	Location Loc;
    PathEntry():Loc() {}
    PathEntry(Location loc):Loc(loc) { }
    bool operator== (const PathEntry& other) const
    {
        return (this->Loc == other.Loc);
    }
};

typedef vector<PathEntry> Path;
std::ostream& operator<<(std::ostream& os, const Path& path);
bool isSamePath(const Path& p1, const Path& p2);

struct IterationStats
{
    int sum_of_costs;
    double runtime;
    int num_of_agents;
    string algorithm;
    int sum_of_costs_lowerbound;
    int num_of_colliding_pairs;
    std::vector<Path> paths;
    IterationStats(int num_of_agents, int sum_of_costs, double runtime, const string& algorithm,
                   int sum_of_costs_lowerbound = 0, int num_of_colliding_pairs = 0, std::vector<Path> paths = {}) :
            num_of_agents(num_of_agents), sum_of_costs(sum_of_costs), runtime(runtime),
            sum_of_costs_lowerbound(sum_of_costs_lowerbound), algorithm(algorithm),
            num_of_colliding_pairs(num_of_colliding_pairs), paths(paths) {}
};

struct PIBTPPS_option{
    int windowSize ;
    bool winPIBTSoft ;
    int timestepLimit ;
};


// Only for three-tuples of std::hash-able types for simplicity.
// You can of course template this struct to allow other hash functions
/*struct three_tuple_hash {
    template <class T1, class T2, class T3>
    std::size_t operator () (const std::tuple<T1, T2, T3> &p) const {
        auto h1 = std::hash<T1>{}(get<0>(p));
        auto h2 = std::hash<T2>{}(get<1>(p));
        auto h3 = std::hash<T3>{}(get<2>(p));
        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2 ^ h3;
    }
};*/

};

namespace std {
template <>
struct hash<ConstrainedGraph::Location>
{
    size_t operator()(const ConstrainedGraph::Location& l) const
    {
        auto hash1 = hash<int>{}(l.location);
        auto hash2 = hash<int>{}(l.index);

        if (hash1 != hash2) {
            return hash1 ^ hash2;             
        }
        
        // If hash1 == hash2, their XOR is zero.
        return hash1;
    }
};

};