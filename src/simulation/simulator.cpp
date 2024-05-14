#include "simulation/simulator.h"
#include "Algorithm/graph.h"
#include "graph/generate_ADG.h"

Simulator::Simulator(const shared_ptr<Graph> & adg): adg(adg) {
  vector<int> init_states(adg->get_num_agents(), 0);
  states = init_states;
}

Simulator::Simulator(const shared_ptr<Graph> & adg, vector<int> visited_states): adg(adg), states(visited_states) {}

// [Q] rivers: what is switchCheck?
int Simulator::checkMovable(vector<int>& movable, bool switchCheck) {
  int timeSpent = 0;
  int agentCnt = adg->get_num_agents();
  for (int agent = 0; agent < agentCnt; agent++) {
    int state = states[agent];
    if (state >= adg->get_num_states(agent) - 1) {
      continue;
    }
    timeSpent += 1;
    int next_state = state + 1;

    vector<pair<int, int>> dependencies;
    if (!switchCheck) {
      dependencies = adg->get_non_switchable_type2_in_neighbor_pairs(agent, next_state);
    } else {
      dependencies = adg->get_type2_in_neighbor_pairs(agent, next_state);
    }

    movable[agent] = 1;
    for (auto & dependency: dependencies) {
      int dep_agent = get<0>(dependency);
      int dep_state = get<1>(dependency);
      
      if (dep_agent != agent) {
        if (dep_state > states[dep_agent]) {
          movable[agent] = 0;
          break;
        }
      }
    }
  }
  return timeSpent;
}

int Simulator::step(bool switchCheck) {
  int agentCnt = adg->get_num_agents();
  vector<int> movable(agentCnt, 0);
  int timeSpent = checkMovable(movable, switchCheck);
  int moveCnt = 0;

  for (int agent = 0; agent < agentCnt; agent++) {
    if (movable[agent] == 1) {
      states[agent] += 1;
      moveCnt += 1;
    }
  }
  if (moveCnt == 0 && timeSpent != 0) {
    cout << "err: "<<moveCnt<<" "<<timeSpent<<endl<<endl;
    Paths &paths = *adg->paths;
    int agentCnt = adg->get_num_agents();
    for (auto agent=0;agent<agentCnt;++agent) {
      auto curr_state=states[agent];
      auto goal_state=adg->get_num_states(agent)-1;
      if (curr_state!=goal_state) {
        cout<<agent<<": "<<states[agent]<<" "<<adg->get_num_states(agent)-1<<endl;
        Location new_location = get<0>((paths[agent])[(states[agent])]);
        Location next_location = get<0>((paths[agent])[(states[agent]+1)]);
        cout<<new_location.first<<","<<new_location.second<<"->"<<next_location.first<<","<<next_location.second<<endl;
        auto && in_pairs=adg->get_type2_in_neighbor_pairs(agent, curr_state+1);
        cout<<"depenencies:"<<endl;
        for (auto & in_pair: in_pairs) {
          cout<<get<0>(in_pair)<<" "<<get<1>(in_pair)<<" = "<<adg->get_global_state_id(in_pair.first,in_pair.second)<<endl;
        }
        cout<<endl;
      }
    }
    return -1;
  }
  return timeSpent;
}

void Simulator::print_location(ofstream &outFile, Location location) {
  int i = get<0>(location);
  int j = get<1>(location);
  outFile << "(" << i << "," << j << ")->";
}

int Simulator::print_soln(const char* outFileName) {
  ofstream outFile;
  outFile.open(outFileName);
  int totalSpend = 0;
  int stepSpend = 0;

  if (outFile.is_open()) {
    vector<vector<Location>> expanded_paths;
    int agentCnt = adg->get_num_agents();
    Paths &paths = *adg->paths;

    for (int agent = 0; agent < agentCnt; agent ++) {
      vector<Location> expanded_path;
      expanded_path.push_back(get<0>((paths[agent])[0]));
      expanded_paths.push_back(expanded_path);
    }

    stepSpend = step(true);
    while (stepSpend != 0) {
      if (stepSpend<0) {
        std::cout<<"fail in print_soln(outFileName)"<<std::endl;
        exit(2);
      }
      // output how many agents take actions at the step.
      // outFile << "step=" << stepSpend << "\n";
      for (int agent = 0; agent < agentCnt; agent ++) {
        Location new_location = get<0>((paths[agent])[(states[agent])]);
        if (!((new_location==expanded_paths[agent].back()) && 
            ((size_t)(states[agent]) == (paths[agent]).size() - 1))) {
          (expanded_paths[agent]).push_back(new_location);
        }
      }
      totalSpend += stepSpend;
      stepSpend = step(true);
    }

    for (int agent = 0; agent < agentCnt; agent ++) {
      // output each agent's modified path.
      outFile << "Agent " << agent << ": ";
      vector<Location> &expanded_path = expanded_paths[agent];
      for (Location location: expanded_path) {
        print_location(outFile, location);
      }
      outFile << std::endl;
    }
    outFile.close();
  }

  return totalSpend;
}

int Simulator::print_soln() {
  int totalSpend = 0;
  int stepSpend = 0;

  stepSpend = step(true);
  while (stepSpend != 0) {
    if (stepSpend<0) {
      std::cout<<"fail in print_soln()"<<std::endl;
      exit(2);
    }
    totalSpend += stepSpend;
    stepSpend = step(true);
  }
  
  return totalSpend;
}