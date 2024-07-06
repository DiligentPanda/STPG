#include "constrained_map_loader.h"
#include <iostream>
#include <fstream>
#include<boost/tokenizer.hpp>
#include <cstring>
#include <bitset>
#include <memory>
#include <tuple>
#include <utility>
#include "nlohmann/json.hpp"

using namespace boost;
using namespace std;

using json=nlohmann::json;

vector<pair<Location, int>> ConstrainedMapLoader::get_transitions(Location loc, int heading, int noWait) const {
  vector<pair<Location, int>> transitions;
  if (use_icg) {
    for (auto &p: icgGraphMaps[agent_idx].at(loc)) {
      transitions.emplace_back(p, -1);
    }
  } else {
    for (auto &p: cgGraphMaps[agent_idx].at(loc)) {
      transitions.emplace_back(p, -1);
    }
  }
  return transitions;
}

// <coordinates>
typedef std::pair<int, int> _Location;
// <location, timestep>
typedef std::vector<pair<_Location, int>> _Path;
// paths for all agents
typedef std::vector<_Path> _Paths;

bool same_locations(_Location location1, _Location location2) {
  int i1 = location1.first;
  int j1 = location1.second;
  int i2 = location2.first;
  int j2 = location2.second;
  
  return (i1 == i2 && j1 == j2);
}

// Return path and stateCnt of an agent
std::tuple<_Path, int> parse_path(string line) {
  int i, j, stateCnt = 0;
  int time = 0;
  size_t comma_pos, leftPar_pos, rightPar_pos;
  _Path path;
  _Location prev_location = std::make_pair(-1, -1);

  while ((leftPar_pos = line.find("(")) != string::npos) {
    // Process an index pair
    comma_pos = line.find(",");
    i = std::stoi(line.substr(leftPar_pos + 1, comma_pos));
    rightPar_pos = line.find(")");
    j = std::stoi(line.substr(comma_pos + 1, rightPar_pos));
    line.erase(0, rightPar_pos + 1);

    // Create a location tuple and add it to the path
    _Location location = std::make_pair(i, j);
    if (!same_locations(location, prev_location)) {
      stateCnt ++;
      path.push_back(make_pair(location, time));
      prev_location = location;
    }
    time++;
  }
  return std::make_tuple(path, stateCnt);
}

// Return all paths, accumulated counts of states, and States
std::shared_ptr<_Paths> parse_soln(const string & fileName) {
  auto paths_ptr=std::make_shared<_Paths>();
  auto & paths = * paths_ptr;

  string fileName_string = fileName;
  ifstream file(fileName_string);
  if (file.is_open()) {
    string line;
    while (getline(file, line)) {
      // Sanity check that the line is a path
      if (line[0] == 'A') {
        std::tuple<_Path, int> parse_result = parse_path(line);
        _Path path = std::get<0>(parse_result);
        // Done with the agent
        paths.push_back(path);
      }
    }
    file.close();
  } else {
    std::cout << "exit\n";
    exit(0);
  }
  return paths_ptr;
}


ConstrainedMapLoader::ConstrainedMapLoader(
  const std::string & map_fp, 
  const std::string & path_fp,
  const std::string & sit_fp
):MapLoader(map_fp) {

    // read in states and delay steps
    std::ifstream in(sit_fp);
    json data=json::parse(in);

    states=data.at("states").get<vector<int> >();
    delay_steps=data.at("delay_steps").get<vector<int> >();

    int num_of_agents=(int) states.size();

    postDelayPlan.resize(num_of_agents);

    auto compressed_paths_ptr = parse_soln(path_fp.c_str());

    for (int a=0; a<num_of_agents;++a) {
        auto & compressed_path=compressed_paths_ptr->at(a);

        int start=states[a];
        int delay_step = delay_steps[a];
        
        int idx=0;
        for (int k=0;k<delay_step;k++)
        {
          int location=compressed_path[start].first.first*cols+compressed_path[start].first.second;
          Location loc(location, idx);
          postDelayPlan[a].emplace_back(loc);
          ++idx;
        }

        for (int j = start; j < compressed_path.size(); j++)
        {
          int location=compressed_path[j].first.first*cols+compressed_path[j].first.second;
          Location loc(location, idx);
          postDelayPlan[a].emplace_back(loc);
          ++idx;
        }
    }

    if (use_icg) {
      fillicgGraphMap();
    } else {
      fillcgGraphMap();
    }
}

void ConstrainedMapLoader::fillcgGraphMap() {
  size_t num_of_agents=postDelayPlan.size();

  cgGraphMaps.clear();
  cgGraphMaps.resize(num_of_agents, {});
  for (int a = 0; a < postDelayPlan.size(); a++) 
  {
    for (int k = 0; k < postDelayPlan[a].size(); k++)
    {
      Location curr = postDelayPlan[a][k].Loc;
      if ( postDelayPlan[a].size()-1 > (curr.index ) ) {
        cgGraphMaps[a].insert({curr, {postDelayPlan[a][curr.index + 1].Loc, curr}});
      } else if (curr.index < delay_steps[a]) {
        cgGraphMaps[a].insert({curr, {postDelayPlan[a][curr.index + 1].Loc}});
      } else {
        cgGraphMaps[a].insert({curr, {curr}});
      }
    }
  }
}

void ConstrainedMapLoader::fillicgGraphMap() {
  size_t num_of_agents=postDelayPlan.size();

  calcRepresentativePoints();

  icgGraphMaps.clear();
  icgGraphMaps.resize(num_of_agents, {});
  
  // give every location in original_plan a list of neighbors, 
	// which change if it is a rep. point
	for (int a = 0; a < postDelayPlan.size(); a++)
	{
		for (int k = 0; k < postDelayPlan[a].size(); k++)
		{
			// std::cout << originalPlan[a][k].Loc.location << ", " << originalPlan[a][k].Loc.index;
			Location curr = postDelayPlan[a][k].Loc;
			if (std::find_if(rep_points_.begin(), rep_points_.end(),
    			[curr](const Location& l) { return (l.location == curr.location);}) == rep_points_.end())
			{
				// not a rep point, must move on if possible
				if (postDelayPlan[a].back().Loc.index > curr.index)
				{
					// must move on, add to unordered map
					icgGraphMaps[a].insert({curr, {postDelayPlan[a][k + 1].Loc}});
				}
				else
				{
					// at end of path, must self transition
					icgGraphMaps[a].insert({curr, {curr}});
				}
			}
			else
			{
				// curr IS a rep point, either move to next OR self transition
				if (postDelayPlan[a].back().Loc.index > curr.index)
					icgGraphMaps[a].insert({curr, {postDelayPlan[a][k + 1].Loc, curr}});
				else
					icgGraphMaps[a].insert({curr, {curr}});
			}
		}
	}
}

void ConstrainedMapLoader::calcRepresentativePoints() {
  std::vector<Location> intersecting_points{};
	for (auto itr = postDelayPlan.begin(); itr != postDelayPlan.end(); itr++)
	{
		for (auto itr2 = postDelayPlan.begin(); itr2 != postDelayPlan.end(); itr2++)
		{
			if (itr != itr2)
			{
				// iterate through the two paths (itr* and itr2*) 
				for (auto pp1 = itr->begin(); pp1 != itr->end(); pp1++)
				{
					for (auto pp2 = itr2->begin(); pp2 != itr2->end(); pp2++)
					{
						if (((*pp1).Loc.location ==  (*pp2).Loc.location) && 
							((*pp1).Loc.location ==  (*pp2).Loc.location))
						{
							auto toFind = (*pp1).Loc;
							if (std::find_if(intersecting_points.begin(), intersecting_points.end(),
    							[&toFind](const Location& l) { return l.location == toFind.location;}) == intersecting_points.end())
								intersecting_points.push_back( (*pp1).Loc );
						}
					}
				}
			}
		}
	}

	if (!intersecting_points.empty())
	{
		// // sort based on time 
		// std::sort(intersecting_points.begin(), intersecting_points.end(), [](const auto& l, const auto& r){
    	// 	return (*l).index < (*r).index;});

		// add a representative point between each of the intersecting points
		// and add a representative point at each intersecting point
		for (auto i = intersecting_points.begin(); i != intersecting_points.end(); i++)
		{
			// only test i if not already inside rep_point
			auto toFind = *i;
			if (std::find_if(rep_points_.begin(), rep_points_.end(),
    			[&toFind](const Location& l) { return l.location == toFind.location;}) == rep_points_.end())
			{
				// add every intersecting point to rep point
				rep_points_.push_back(*i);

				// now add a rep. point between this intersecting point and the next one
				// do this for every path that consists of intersecting_point i

				for (auto pathPtr = postDelayPlan.begin(); pathPtr != postDelayPlan.end(); pathPtr++)
				{
					// for every path, find if Location i is inside
					for (auto pp = pathPtr->begin(); pp != pathPtr->end(); pp++)
					{
						if ( (*pp).Loc.location == (*i).location )
						{
							// found current intersection point, now see if next point is in intersecting
							// add next point to rep points if it is NOT in intersection points
							auto pp_nxt = std::next(pp);
							if (std::find_if(intersecting_points.begin(), intersecting_points.end(),
    							[pp_nxt](const Location& l) { return l.location == (*pp_nxt).Loc.location;}) == intersecting_points.end())
							{
								// pp_nxt is NOT an intersection point, now add to rep point
								rep_points_.push_back((*pp_nxt).Loc);
							}
							// 
							// for (auto i_nxt = intersecting_points.begin(); i_nxt != intersecting_points.end(); i_nxt++)
							// {
							// 	if ( (*pp_nxt).Loc.location == (*i_nxt).location )
							// 	{
							// 		// finally, can add pp_nxt.Loc as rep point
							// 		add_pp_nxt = false;
							// 		break;
							// 	}
							// }
							// if (!add_pp_nxt)
							// 	rep_points_.push_back((*pp_nxt).Loc);
						}
					}

				}
			}
		}
		assert(intersecting_points.size() <= rep_points_.size());
		// std::cout << "int size: " << intersecting_points.size() << std::endl;
		// std::cout << "rep size: " << rep_points_.size() << std::endl;
	}
	
	// add new start locations rep points as well
	for (auto & path: postDelayPlan)
	{
		auto & toFind = path[0].Loc;
		if (std::find_if(rep_points_.begin(), rep_points_.end(),
    			[&toFind](const Location& l) { return l.location == toFind.location;}) == rep_points_.end())
			rep_points_.push_back(toFind);
	}
}