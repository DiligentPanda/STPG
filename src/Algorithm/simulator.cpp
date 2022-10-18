#include "simulator.h"

Simulator::Simulator(ADG input_adg) {
  adg = input_adg;
  vector<int> init_states(get_agentCnt(adg), 0);
  states = init_states;
}

Simulator::Simulator(ADG input_adg, vector<int> visited_states) {
  adg = input_adg;
  int agentCnt = get_agentCnt(adg);
  vector<int> new_states;
  for (int agent = 0; agent < agentCnt; agent ++) {
    new_states.push_back(visited_states[agent]);
  }
  states = new_states;
}

bool Simulator::move(vector<int>& moved, int agent, int *timeSpent, bool switchCheck) {
  if (moved[agent] == 1) {
    return false;
  }
  moved[agent] = 1;
  int state = states[agent];
  if (state >= get_stateCnt(adg, agent) - 1) {
    return false;
  }
  timeSpent[0] += 1;
  int next_state = state + 1;

  if (switchCheck) {
    // Sanity check: no more switchable in neighbors 
    vector<pair<int, int>> switchables = get_switchable_inNeibPair(adg, agent, next_state);
    assert(switchables.size() == 0);
  }

  vector<pair<int, int>> dependencies = get_nonSwitchable_inNeibPair(adg, agent, next_state);
  for (pair<int, int> dependency: dependencies) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);
    
    if (dep_agent != agent) {
      if (dep_state > states[dep_agent]) {
        return false;
      } else if (dep_state == states[dep_agent]) {
        if (!move(moved, dep_agent, timeSpent, switchCheck)) {
          return false;
        }
      }
    }
  }
  // All nonSwitchable dependencies resolved
  timeSpent[1] += 1;
  states[agent] += 1;
  return true;
}

int Simulator::step(bool switchCheck) {
  int timeSpent[2] = {0};
  int agentCnt = get_agentCnt(adg);
  vector<int> moved(agentCnt, 0);
  for (int agent = 0; agent < agentCnt; agent++) {
    move(moved, agent, timeSpent, switchCheck);
  }
  if (timeSpent[1] == 0 && timeSpent[0] != 0) {
    
    int cnt = 0;
    for (int state: states) {
      std::cout << cnt << ": " << get_stateCnt(adg, cnt) - 1 -state << ";  ";
      cnt ++;
    }
    std::cout <<  "\n" ;
    std::cout << "no progress\n";
  }
  return timeSpent[0];
}

tuple<int, int, int, int> Simulator::detectSwitch() {
  int agentCnt = get_agentCnt(adg);
  for (int agent = 0; agent < agentCnt; agent++) {
    int state = states[agent];
    int next_state = state + 1;
    if (next_state < get_stateCnt(adg, agent)) {
      vector<pair<int, int>> dependencies = get_switchable_inNeibPair(adg, agent, next_state);

      for (pair<int, int> dependency: dependencies) {
        int dep_agent = get<0>(dependency);
        int dep_state = get<1>(dependency);
        
        assert(dep_agent != agent);
        if (dep_state > states[dep_agent]) {
          return make_tuple(dep_agent, dep_state, agent, next_state);
        } else {
          fix_type2_edge(adg, dep_agent, dep_state, agent, next_state);
        }
      }
    }
  }
  // No switchable edge detected
  return make_tuple(-1, -1, -1, -1);
}

// bool Simulator::move_print(vector<int>& moved, int agent, int *timeSpent) {
//   if (moved[agent] == 1) {
//     return false;
//   }
//   moved[agent] = 1;
//   int state = states[agent];
//   if (state >= get_stateCnt(adg, agent) - 1) {
//     return false;
//   }
//   timeSpent[0] += 1;
//   int next_state = state + 1;

//   if (switchCheck) {
//     // Sanity check: no more switchable in neighbors 
//     vector<pair<int, int>> switchables = get_switchable_inNeibPair(adg, agent, next_state);
//     assert(switchables.size() == 0);
//   }

//   vector<pair<int, int>> dependencies = get_nonSwitchable_inNeibPair(adg, agent, next_state);
//   for (pair<int, int> dependency: dependencies) {
//     int dep_agent = get<0>(dependency);
//     int dep_state = get<1>(dependency);
    
//     if (dep_agent != agent) {
//       if (dep_state > states[dep_agent]) {
//         return false;
//       } else if (dep_state == states[dep_agent]) {
//         if (!move(moved, dep_agent, timeSpent, switchCheck)) {
//           return false;
//         }
//       }
//     }
//   }
//   // All nonSwitchable dependencies resolved
//   timeSpent[1] += 1;
//   states[agent] += 1;
//   return true;
// }

// int Simulator::step_print(vector<vector<Location>> res_paths) {
//   int timeSpent[2] = {0};
//   int agentCnt = get_agentCnt(adg);
//   vector<int> moved(agentCnt, 0);
//   for (int agent = 0; agent < agentCnt; agent++) {
//     move_print(moved, agent, timeSpent);
//   }
//   if (timeSpent[1] == 0 && timeSpent[0] != 0) {
    
//     int cnt = 0;
//     for (int state: states) {
//       std::cout << cnt << ": " << get_stateCnt(adg, cnt) - 1 -state << ";  ";
//       cnt ++;
//     }
//     std::cout <<  "\n" ;
//     std::cout << "no progress\n";
//   }
//   return timeSpent[0];
// }

// int main(int argc, char** argv) {
//   char* fileName = argv[1];
//   ADG adg = construct_ADG(fileName);
//   ADG res = Astar(adg);
//   print_graph(get<0>(res));
//   // for (pair<int, int> as: get_switchable_inNeibPair(adg, 13, 1)) {
//   //   std::cout << get<0>(as) << ", " << get<1>(as) << ";  ";
//   // }
//   // Simulator simulator(adg);

//   // int agent1, state1, agent2, state2;
//   // int step = 0;
//   // int g = 0;
//   // tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
//   // while (agent1 < 0) {
//   //   int g_step = simulator.step(true);
//   //   if (g_step == 0) {
//   //     std::cout << "reach goal\n";
//   //     return 0;
//   //   }
//   //   step ++;
//   //   g += g_step;

//   //   tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
//   // }
//   // if (agent1 >= 0) // Detected a switchable edge
//   // {
//   //   std::cout << "detected switchable: " << agent1 << ", " << state1 << ";  " << agent2 << ", " << state2 << "\n";
//   //   std::cout << "step = " << step << ", g = " << g << "\n";
//   // }

//   return 0;
// }