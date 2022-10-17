#include "simulator.h"

Simulator::Simulator(ADG adg) {
  adg = adg;
  int states[get_agentCnt(adg)] = {0};
}

bool Simulator::move(int *moved, int agent, int *timeSpent) {
  if (moved[agent] == 1) return false;
  moved[agent] = 1;
  int state = states[agent];
  if (state >= get_stateCnt(adg, agent) - 1) return false;

  timeSpent[0] += 1;
  int next_state = state + 1;
  vector<pair<int, int>> dependencies = get_type2_inNeibPair(adg, agent, next_state);

  for (pair<int, int> dependency: dependencies) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);

    assert(dep_agent != agent);
    if (dep_state > states[dep_agent]) {
      return false;
    } else if (dep_state == state[dep_agent]) {
      if (!move(moved, dep_agent, timeSpent)) {
        return false;
      }
    }
  }
  // All type2 dependencies resolved
  states[agent] += 1;
  return true;
}

int Simulator::step() {
  int timeSpent[1] = {0};
  int agentCnt = get_agentCnt(adg);
  int moved[agentCnt] = {0};
  for (int agent = 0; agent < agentCnt; agent++) {
    move(moved, agent, timeSpent);
  }
  return timeSpent[0];
}