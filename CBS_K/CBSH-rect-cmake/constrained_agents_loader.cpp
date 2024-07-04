//=======================================================================

#include "constrained_agents_loader.h"
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <fstream>
#include<boost/tokenizer.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <utility>
#include <algorithm>  // for remove_if
#include <ctime>
using namespace boost;
using namespace std;

const int RANDOM_WALK_STEPS = 100000;

ConstrainedAgentsLoader::ConstrainedAgentsLoader() {}

ConstrainedAgentsLoader::ConstrainedAgentsLoader(ConstrainedMapLoader & constrained_map_loader, int max_k, bool diff_k) {
    if (diff_k) {
      std::cout<<"not supported"<<std::endl;
    }

    this->num_of_agents=constrained_map_loader.postDelayPlan.size();
    this->max_k = max_k;

    departure_times.resize(num_of_agents,0);
    headings.resize(num_of_agents,-1);
    min_end_time.resize(num_of_agents,0);
    done.resize(num_of_agents,false);
    k.resize(num_of_agents,this->max_k); 
    initial_locations.clear();
    goal_locations.clear();
    
    for (int i = 0; i < num_of_agents; i++) {
      initial_locations.push_back(constrained_map_loader.postDelayPlan[i].front().Loc);
      goal_locations.push_back(constrained_map_loader.postDelayPlan[i].back().Loc);
    }
}

// void ConstrainedAgentsLoader::printAgentsInitGoal () {
//   cout << "AGENTS:" << endl;;
//   for (int i=0; i<num_of_agents; i++) {
//     cout << "Agent" << i << " : I=(" << initial_locations[i].first << "," << initial_locations[i].second << ") ; G=(" <<
//       goal_locations[i].first << "," << goal_locations[i].second << ")" << ", k: "<< k[i]<<", departure_time: "<<departure_times[i]<<
//       ", min_end_time: "<<min_end_time[i]<<", done: "
//       << done[i] <<", Heading: "<<headings[i]<< endl;
//   }
//   cout << endl;
// }

// returns the agents' ids if they occupy [row,col] (first for start, second for goal)
// pair<int, int> ConstrainedAgentsLoader::agentStartOrGoalAt(int row, int col) {
//   int f = -1;
//   int s = -1;
//   for (vector< pair<int, int> >::iterator it = initial_locations.begin(); it != initial_locations.end(); ++it)
//     if ( it->first == row && it->second == col )
//       f = std::distance(initial_locations.begin(), it);
//   for (vector< pair<int, int> >::iterator it = goal_locations.begin(); it != goal_locations.end(); ++it)
//     if ( it->first == row && it->second == col )
//       s = std::distance(goal_locations.begin(), it);
//   return make_pair(f, s);
// }


// void ConstrainedAgentsLoader::clearLocationFromAgents(int row, int col) {
//   pair<int, int> idxs = agentStartOrGoalAt(row, col);
//   if ( idxs.first != -1 ) {  // remove the agent who's start is at [row,col]
//     initial_locations.erase( initial_locations.begin() + idxs.first );
//     goal_locations.erase ( goal_locations.begin() + idxs.first );
//     num_of_agents--;
//   }
//   idxs = agentStartOrGoalAt(row, col);
//   if ( idxs.second != -1 ) {  // remove the agent who's goal is at [row,col]
//     initial_locations.erase( initial_locations.begin() + idxs.second );
//     goal_locations.erase( goal_locations.begin() + idxs.second );
//     num_of_agents--;
//   }
// }


// // add an agent
// void ConstrainedAgentsLoader::addAgent(int start_row, int start_col, int goal_row, int goal_col,int min_time,int finish, int heading) {
//   this->initial_locations.push_back(make_pair(start_row, start_col));
//   this->goal_locations.push_back(make_pair(goal_row, goal_col));
//   this->headings.push_back(heading);
//   this->min_end_time.push_back(min_time);
//   this->done.push_back(finish);
//   num_of_agents++;
// }

// void ConstrainedAgentsLoader::saveToFile(std::string fname) {
//   ofstream myfile;
//   myfile.open(fname);
//   myfile << num_of_agents << endl;
//   for (int i = 0; i < num_of_agents; i++)
//     myfile << initial_locations[i].first << "," << initial_locations[i].second << ","
//            << goal_locations[i].first << "," << goal_locations[i].second << ","
// 		   /*<< max_v[i] << "," << max_a[i] << "," << max_w[i] << ","*/  << endl;
//   myfile.close();
// }

// void ConstrainedAgentsLoader::clear(){
//     initial_locations.clear();
//     goal_locations.clear();
//     min_end_time.clear();
//     done.clear();
//     headings.clear();
//     num_of_agents = 0;
// }
