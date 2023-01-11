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

int Simulator::checkMovable(vector<int>& movable) {
  int timeSpent = 0;
  int agentCnt = get_agentCnt(adg);
  for (int agent = 0; agent < agentCnt; agent++) {
    int state = states[agent];
    if (state >= get_stateCnt(adg, agent) - 1) {
      continue;
    }
    timeSpent += 1;
    int next_state = state + 1;

    vector<pair<int, int>> dependencies = get_nonSwitchable_inNeibPair(adg, agent, next_state);
    for (pair<int, int> dependency: dependencies) {
      int dep_agent = get<0>(dependency);
      int dep_state = get<1>(dependency);
      
      if (dep_agent != agent) {
        if (dep_state > states[dep_agent]) {
          continue;
        }
      }
    }
    movable[agent] = 1;
  }
  return timeSpent;
}

int Simulator::step(bool switchCheck) {
  int agentCnt = get_agentCnt(adg);
  vector<int> movable(agentCnt, 0);
  int timeSpent = checkMovable(movable);
  int moveCnt = 0;

  for (int agent = 0; agent < agentCnt; agent++) {
    if (movable[agent] == 1) {
      states[agent] += 1;
      moveCnt += 1;
    }
  }
  if (moveCnt == 0 && timeSpent != 0) {
    return -1;
  }
  return timeSpent;
}