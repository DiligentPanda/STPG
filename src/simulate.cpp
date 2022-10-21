#include <random>
#include <fstream>

#include "Algorithm/Astar.h"

bool Simulator::amove(vector<int>& moved, int agent, int *timeSpent, int *delayer, int p, int d) {
  if (moved[agent] == 1) {
    return false;
  }

  moved[agent] = 1;
  int state = states[agent];
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

  vector<pair<int, int>> dependencies_ns = get_nonSwitchable_inNeibPair(adg, agent, next_state);
  vector<pair<int, int>> dependencies_s = get_switchable_inNeibPair(adg, agent, next_state);
  for (pair<int, int> dependency: dependencies_ns) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);
    
    if (dep_agent != agent) {
      if (dep_state > states[dep_agent]) {
        return false;
      } else if (dep_state == states[dep_agent]) {
        if (!amove(moved, dep_agent, timeSpent, delayer, p, d)) {
          return false;
        }
      }
    }
  }

  for (pair<int, int> dependency: dependencies_s) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);
    
    if (dep_state > states[dep_agent]) {
      return false;
    } else if (dep_state == states[dep_agent]) {
      if (!amove(moved, dep_agent, timeSpent, delayer, p, d)) {
        return false;
      }
    }
    assert(get_type2_nonSwitchable_edge(get<0>(adg), compute_vertex(get<2>(adg), dep_agent, dep_state), compute_vertex(get<2>(adg), agent, next_state)));
  }

  // Going to move. fix all outgoing switchable edges
  for (pair<int, int> dependency: get_switchable_outNeibPair(adg, agent, next_state)) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);
    fix_type2_edge(adg, agent, next_state, dep_agent, dep_state);
  }

  // All nonSwitchable dependencies resolved
  timeSpent[1] += 1;
  states[agent] += 1;
  return true;
}

int Simulator::astep(int p, int d, int *delayer) {
  int timeSpent[2] = {0};
  int agentCnt = get_agentCnt(adg);
  vector<int> moved(agentCnt, 0);

  for (int agent = 0; agent < agentCnt; agent++) {
    amove(moved, agent, timeSpent, delayer, p, d);
  }

  if (timeSpent[1] == 0 && timeSpent[0] != 0) {
    return -1;
  }
  return timeSpent[0];
}

void print_for_replanning(ADG adg, vector<int> states, ofstream &outFile) {
  outFile << "version 1"<< std::endl;;
  for (int agent = 0; agent < get_agentCnt(adg); agent ++) {
    Location current = get_state_target(adg, agent, states[agent]);
    int i1 = get<1>(current);
    int j1 = get<0>(current);
    Location goal = get_state_target(adg, agent, get_stateCnt(adg, agent) - 1);
    int i2 = get<1>(goal);
    int j2 = get<0>(goal);
    outFile << "0\trandom-32-32-20.map\t32\t32\t" << i1 << "\t" << j1 << "\t" << i2 << "\t" << j2 << "\t0.00"<< std::endl;;
  }
}

int Simulator::asimulate(int p, int d, ofstream &outFile) {
  int stepSpend = 0;
  int totalSpend = 0;
  int delayer[2] = {-1, 0};

  stepSpend = astep(p, d, delayer);
  while (stepSpend != 0) {
    if (delayer[0] >= 0 && delayer[1] == 0) // a delay just ends
    {
      Simulator simulator_replan(adg, states);
      ADG adg_copy = copy_ADG(adg);
      Simulator simulator_original(adg_copy, states);
      set_switchable_nonSwitchable(get<0>(simulator_original.adg));
      int originalTime = heuristic(simulator_original);
      std::cout << "original time spend = " << originalTime << "\n\n";

      // TODO: NOW TESTING FOR ONLY DELAY ONCE!!!!
      microseconds timer(0);
      auto start = high_resolution_clock::now();
      ADG replanned_adg = Astar(simulator_replan);
      auto stop = high_resolution_clock::now();
      auto duration = duration_cast<microseconds>(stop - start);
      timer += duration;
      std::cout << "------------------------REPLANNING TAKE TIME:::: " << timer.count() << "\n\n";
      
      Simulator simulator_res(replanned_adg, states);
      int timeSum = heuristic(simulator_res);
      std::cout << "solution time spend = " << timeSum << "\n\n";

      print_for_replanning(adg, states, outFile);
      delayer[0] = -1; // set to indicate not immediately after a delay
      exit(0);
    }

    if (stepSpend < 0) return -1; // stuck
    totalSpend += stepSpend;
    stepSpend = astep(p, d, delayer);
  }
  return totalSpend;
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  int p = atoi(argv[2]);
  int d = atoi(argv[3]);
  const char* outFileName = argv[4];

  ADG adg = construct_ADG(fileName);
  for (int agent = 0; agent < get_agentCnt(adg); agent++) {
    for (pair<int, int> outNeigb: get_switchable_outNeibPair(adg, agent, 0)) {
      // Fix starting edge
      fix_type2_edge(adg, agent, 0, get<0>(outNeigb), get<1>(outNeigb));
    }

    for (pair<int, int> inNeigb: get_switchable_inNeibPair(adg, agent, get_stateCnt(adg, agent)-1)) {
      // Fix ending edge
      fix_type2_edge(adg, get<0>(inNeigb), get<1>(inNeigb), agent, get_stateCnt(adg, agent)-1);
    }
  }

  ofstream outFile;
  outFile.open(outFileName);
  if (outFile.is_open()) {
    Simulator simulator(adg);
    simulator.asimulate(p, d, outFile);
    outFile.close();
  }
  return 0;
}