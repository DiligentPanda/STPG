#include <random>
#include "Algorithm/Astar.h"

bool move(Simulator simulator, vector<int>& moved, int agent, int *timeSpent, int *delayer, int p, int d) {
  if (moved[agent] == 1) {
    return false;
  }

  moved[agent] = 1;
  int state = (simulator.states)[agent];
  if (state >= get_stateCnt(adg, agent) - 1) {
    return false;
  }
  timeSpent[0] += 1;

  if (delayer[0] == -1) // Not within a delay
  {
    random_device rd;  
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, 100);
    if (distrib(gen) <= p) {
      delayer[0] = agent;
      delayer[1] = d;
    }
  }

  if (delayer[0] == agent && delayer[1] > 0) {
    delayer[1] --;
    return false;
  }

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

int step(Simulator simulator, int p, int d, int *delayer) {
  int timeSpent[2] = {0};
  int agentCnt = get_agentCnt(adg);
  vector<int> moved(agentCnt, 0);

  for (int agent = 0; agent < agentCnt; agent++) {
    move(simulator, moved, agent, timeSpent, delayer, p, d);
  }

  if (delayer[1] > 0) delayer[1] -= 1;
  if (timeSpent[1] == 0 && timeSpent[0] != 0) {
    return -1;
  }
  return timeSpent[0];
}

void print_for_replanning(ADG adg, vector<int> states) {
  std::cout << "version 1\n";
  for (int agent = 0; agent < get_agentCnt(adg); agent ++) {
    Location current = get_state_target(adg, agent, states[agent]);
    int i1 = get<1>(current);
    int j1 = get<0>(current);
    Location goal = get_state_target(adg, agent, get_stateCnt(adg, agent) - 1);
    int i2 = get<1>(goal);
    int j2 = get<0>(goal);
    std::cout << "0\trandom-32-32-20.map\t32\t32\t" << i1 << "\t" << j1 << "\t" << i2 << "\t" << j2 << "\t0.00\n";
  }
}

int simulate(Simulator simulator, int p, int d)  {
  int stepSpend = 0;
  int totalSpend = 0;
  int delayer[2] = {-1, 0};

  stepSpend = step(simulator);
  while (stepSpend != 0) {
    if (delayer[0] >= 0 && delayer[1] == 0) // a delay just ends
    {
      vector<int> record_states = simulator.states;
      // TODO: NOW TESTING FOR ONLY DELAY ONCE!!!!
      microseconds timer(0);
      auto start = high_resolution_clock::now();
      ADG replanned_adg = Astar(simulator);
      auto stop = high_resolution_clock::now();
      auto duration = duration_cast<microseconds>(stop - start);
      timer += duration;
      std::cout << "------------------------REPLANNING TAKE TIME:::: " << timer.count() << "\n\n";
      
      Simulator simulator_res(replanned_adg, record_states);
      int timeSum = heuristic(simulator_res);
      std::cout << "solution time spend = " << timeSum << "\n\n";

      print_for_replanning(adg, record_states);
      delayer[0] = -1; // set to indicate not immediately after a delay
      exit(0);
    }

    if (stepSpend < 0) return -1; // stuck
    totalSpend += stepSpend;
    stepSpend = step(simulator, p, d, delayer);
  }
  return totalSpend;
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  int p = atoi(argv[2]);
  int d = atoi(argv[3]);
  ADG adg = construct_ADG(fileName);
  Simulator simulator(adg);
  simulate(Simulator, p, d);

}