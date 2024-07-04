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
  for (auto &p: cgGraphMaps[agent_idx].at(loc)) {
    transitions.emplace_back(p, -1);
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

    vector<int> states=data.at("states").get<vector<int> >();
    vector<int> delay_steps=data.at("delay_steps").get<vector<int> >();

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

    cgGraphMaps.resize(num_of_agents, {});
    // give every location in original_plan a list of neighbors, 
    // which change if it is a rep. point
    for (int a = 0; a < postDelayPlan.size(); a++) 
    {
      for (int k = 0; k < postDelayPlan[a].size(); k++)
      {
        Location curr = postDelayPlan[a][k].Loc;
        if ( postDelayPlan[a].size()-1 > (curr.index ) ) {
          cgGraphMaps[a].insert({curr, {postDelayPlan[a][curr.index + 1].Loc, curr}});
        } else if (curr.index <= delay_steps[a]) {
          cgGraphMaps[a].insert({curr, {postDelayPlan[a][curr.index + 1].Loc}});
        } else {
          cgGraphMaps[a].insert({curr, {curr}});
        }
      }
    }
}