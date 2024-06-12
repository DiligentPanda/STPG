#pragma once
#include "ConstrainedGraph/Instance.h"
#include <unordered_map>
#include <random>
#include "ConstrainedGraph/common.h"
#include "graph/graph.h"

namespace ConstrainedGraph {

class DelayInstance: public Instance
{
public:
	DelayInstance(const string& map_fname, const string& agent_fname, 
		vector<Path>& paths,
		int num_of_agents = 0, 
        int num_of_delays = 1,
        bool collidingPaths=false, 
        int num_of_rows = 0, 
        int num_of_cols = 0, 
		int num_of_obstacles = 0, 
        int warehouse_width = 0);

    DelayInstance(
        const shared_ptr<Graph> & graph,
        const string& map_fp, 
        const std::string & scen_fp,
        const std::string & path_fp,
        const std::vector<int> & states,
        const std::vector<int> & delay_steps
    );
    Path parse_path(string line);
    std::vector<Path> parse_paths(const string & path_fp);

	~DelayInstance() {};
	bool foundDelay()
	{
        if (!collidingPaths_)
        {
            if (!delay_.empty())
                return true;
            else
                return false;
        }   
        else 
        {
            return true;
        }
	}

    list<Location> getNeighbors(Location curr);

    int getDegree(int loc)
    {
    	auto n = getNeighbors(loc);
    	return n.size();
    }

    void activateImprovement() 
    {
    	improvements_++;
    	calcRepresentativePoints();
    	fillicgGraphMap();
    };

    int num_of_delays_;

private:
	void createDelay();
	bool isCollisionFree();
	void changeStarts();
	list<Location> improvedCG_getNeighbors(Location curr);
	void calcRepresentativePoints();
	void fillicgGraphMap();
	void fillcgGraphMap();
	int improvements_;
    bool collidingPaths_;
	vector<Location> rep_points_;
	// std::vector<bool> isRepPoint;
	vector<boost::unordered_map<Location, list<Location>, hash<Location>>> cgGraphMaps;
	vector<boost::unordered_map<Location, list<Location>, hash<Location>>> icgGraphMaps;
};

};