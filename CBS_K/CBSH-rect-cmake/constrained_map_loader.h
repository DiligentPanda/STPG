// Load's a 2D map.
#pragma once

#include <string>
#include <vector>
#include "common.h"
#include "map_loader.h"
#include <boost/unordered_map.hpp>
#include "Path.h"


namespace ConstrainedGraph {

using namespace std;

class ConstrainedMapLoader: MapLoader
{
 public:
  int agent_idx=-1;
  std::vector<boost::unordered_map<Location, list<Location>, std::hash<Location>>> cgGraphMaps;
  std::vector<Path> postDelayPlan{};

  ConstrainedMapLoader(
    const string& map_fp, 
    const std::string & scen_fp,
    const std::string & path_fp,
    const std::vector<int> & states,
    const std::vector<int> & delay_steps
  ); 
  // load map from file
  // ConstrainedMapLoader(int rows, int cols); // initialize new [rows x cols] empty map
  // ConstrainedMapLoader();

	void set_agent_idx(int _agent_idx) {agent_idx=_agent_idx;}
  std::vector<std::pair<Location, int>> get_transitions(Location loc, int heading, int noWait) const;
  
};

};