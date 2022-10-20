#include "Algorithm/Astar.h"
bool move(Simulator simulator, vector<int>& moved, int agent, int *timeSpent) {
  if (moved[agent] == 1) {
    return false;
  }

  moved[agent] = 1;
  int state = (simulator.states)[agent];
  if (state >= get_stateCnt(adg, agent) - 1) {
    return false;
  }
  timeSpent[0] += 1;

  //here

  int next_state = state + 1;
  vector<pair<int, int>> toFix_in = get_switchable_inNeibPair(adg, agent, next_state);
  vector<pair<int, int>> toFix_out = get_switchable_outNeibPair(adg, agent, next_state);

  for (pair<int, int> dependency: toFix_in) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);
    fix_type2_edge(adg, dep_agent, dep_state, agent, next_state);
  }

  for (pair<int, int> dependency: toFix_out) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);
    fix_type2_edge(adg, agent, next_state, dep_agent, dep_state);
  }

  vector<pair<int, int>> dependencies = get_nonSwitchable_inNeibPair(adg, agent, next_state);
  for (pair<int, int> dependency: dependencies_ns) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);
    
    if (dep_agent != agent) {
      if (dep_state > (simulator.states)[dep_agent]) {
        return false;
      } else if (dep_state == (simulator.states)[dep_agent]) {
        if (!move(simulator, moved, dep_agent, timeSpent, switchCheck)) {
          return false;
        }
      }
    }
  }
  // All nonSwitchable dependencies resolved
  timeSpent[1] += 1;
  (simulator.states)[agent] += 1;
  return true;
}

int step(Simulator simulator) {
  int timeSpent[2] = {0};
  int agentCnt = get_agentCnt(adg);
  vector<int> moved(agentCnt, 0);

  for (int agent = 0; agent < agentCnt; agent++) {
    move(simulator, moved, agent, timeSpent);
  }
  if (timeSpent[1] == 0 && timeSpent[0] != 0) {
    return -1;
  }
  return timeSpent[0];
}

int simulate(Simulator simulator)  {
  int stepSpend = 0;
  int totalSpend = 0;

  stepSpend = step(simulator);
  while (stepSpend != 0) {
    if (stepSpend < 0) return -1; // stuck
    totalSpend += stepSpend;
    stepSpend = step(simulator);
  }
  return totalSpend;
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  ADG adg = construct_ADG(fileName);
  Simulator simulator(adg);
  simulate(Simulator);


  ADG res = Astar(adg);
  std::cout<<"finished, result graph: \n";

  Simulator simulator(res);
  int timeSum = heuristic(simulator);
  std::cout << "solution time spend = " << timeSum << "\n";

  ADG original_adg = construct_ADG(fileName);
  set_switchable_nonSwitchable(get<0>(original_adg));
  Simulator original_simulator(original_adg);
  int original_timeSum = heuristic(original_simulator);
  std::cout << "optimal time spend = " << original_timeSum << "\n";
  return 0;
}