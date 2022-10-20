#include "simulator.h"

Simulator::Simulator(ADG input_adg) {
  adg = input_adg;
  vector<int> init_states(get_agentCnt(adg), 0);
  states = init_states;
}

Simulator::Simulator(ADG input_adg, vector<int> visited_states) {
  adg = input_adg;
  states = visited_states;
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
    return -1;
  }
  return timeSpent[0];
}

tuple<int, int, int, int> Simulator::detectSwitch() {
  int agentCnt = get_agentCnt(adg);
  for (int agent = 0; agent < agentCnt; agent++) {
    int state = states[agent];
    int next_state = state + 1;
    if (next_state < get_stateCnt(adg, agent)) {
      vector<pair<int, int>> dependenciesIn = get_switchable_inNeibPair(adg, agent, next_state);
      vector<pair<int, int>> dependenciesOut = get_switchable_outNeibPair(adg, agent, next_state);

      for (pair<int, int> dependency: dependenciesIn) {
        int dep_agent = get<0>(dependency);
        int dep_state = get<1>(dependency);
        
        assert(dep_agent != agent);
        if (dep_state > states[dep_agent]) {
          return make_tuple(dep_agent, dep_state, agent, next_state);
        } else {
          if (dep_state != 0) {
            std::cout << dep_agent << ", " << dep_state << " -> " << agent << ", " << next_state << "\n";
            assert(false);
          } else {
            fix_type2_edge(adg, dep_agent, dep_state, agent, next_state);
          }
        }
      }
      for (pair<int, int> dependency: dependenciesOut) {
        int dep_agent = get<0>(dependency);
        int dep_state = get<1>(dependency);
        
        assert(dep_agent != agent);
        if (dep_state > states[dep_agent]) {
          return make_tuple(agent, next_state, dep_agent, dep_state);
        } else {
          std::cout << dep_agent << ", " << dep_state << " -> " << agent << ", " << next_state << "\n";
          assert(false);
          fix_type2_edge(adg, agent, next_state, dep_agent, dep_state);
        }
      }
    }
  }
  // No switchable edge detected
  return make_tuple(-1, -1, -1, -1);
}
