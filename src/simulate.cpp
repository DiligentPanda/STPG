#include <random>
#include <fstream>

#include "Algorithm/Astar.h"

int Simulator::step_wdelay(int p, bool *delay_mark, int *delayed_agent) {
  int agentCnt = get_agentCnt(adg);

  random_device rd;  
  mt19937 gen(rd());
  uniform_int_distribution<> distrib(1, 100);

  vector<int> movable(agentCnt, 0);
  int timeSpent = checkMovable(movable);
  int moveCnt = 0;

  if (distrib(gen) <= p) {
    vector<int> candidates;
    for (int agent = 0; agent < agentCnt; agent++) {
      if (movable[agent] == 1) {
        candidates.push_back(agent);
      }
    }
    if (candidates.size() > 0) {
      uniform_int_distribution<> distrib_agent(0, candidates.size()-1);
      *delayed_agent = candidates[distrib_agent(gen)];
      *delay_mark = true;
      return 0;
    }
  }

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

int Simulator::simulate_wdelay(int p, int d, ofstream &outFile, int timeout, bool term_shortcut) {
  int stepSpend = 1;
  bool delay_mark = false;
  int delayed_agent = -1;

  while (stepSpend != 0) {
    stepSpend = step_wdelay(p, &delay_mark, &delayed_agent);
    if (delay_mark) // a delay just happened
    {
      // print_for_replanning(adg, states, outFile);

      int delayed_state = states[delayed_agent];

      microseconds timer_constructADG(0);
      auto start = high_resolution_clock::now();
      ADG adg_delayed = construct_delayed_ADG(adg, d, delayed_agent, delayed_state);
      auto stop = high_resolution_clock::now();
      timer_constructADG += duration_cast<microseconds>(stop - start);

      ADG adg_delayed_copy = copy_ADG(adg_delayed);
      set_switchable_nonSwitchable(get<0>(adg_delayed_copy));
      Simulator simulator_original(adg_delayed_copy);
      int originalTime = simulator_original.print_soln("ori.txt");
      Simulator simulator_ori_trunc(adg_delayed_copy, states);
      int oriTime_trunc = simulator_ori_trunc.print_soln("ori.txt");

      microseconds timer(0);
      start = high_resolution_clock::now();
      Astar search(timeout, term_shortcut);
      ADG replanned_adg = search.startExplore(adg_delayed);
      stop = high_resolution_clock::now();
      timer += duration_cast<microseconds>(stop - start);

      if (duration_cast<seconds>(timer).count() >= timeout) {
        outFile << timer.count() << "," << 
        timer.count() + timer_constructADG.count() << ",,,,,";
        search.print_stats(outFile);
        exit(0);
      }
      
      Simulator simulator_res(replanned_adg);
      int timeSum = simulator_res.print_soln("out.txt");
      Simulator simulator_res_trunc(replanned_adg, states);
      int timeSum_trunc = simulator_res_trunc.print_soln("out.txt");

      outFile << timer.count() << "," << 
      timer.count() + timer_constructADG.count() << "," << 
      originalTime << "," <<
      timeSum << "," << 
      oriTime_trunc << "," <<
      timeSum_trunc << ",";
      
      search.print_stats(outFile);
      exit(0);
    }
    if (stepSpend < 0) return -1; // stuck
  }
  std::cout << "no delay happened\n";
  return 0;
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  int p = atoi(argv[2]);
  int d = atoi(argv[3]);
  const char* outFileName = argv[4];
  int print_header = atoi(argv[5]);
  int timeout = atoi(argv[6]);
  int term_choice = atoi(argv[7]);

  bool term_shortcut = (term_choice == 1);
  ADG adg = construct_ADG(fileName);

  ofstream outFile;
  outFile.open(outFileName, ios::app);
  if (outFile.is_open()) {
    if (print_header) {
      outFile << "search time,search + construct time,original total cost,"	
      << "solution total cost,original remain cost,solution remain cost,"
      << "explored node,pruned node,added node,heuristic time,branch time,"
      << "sort time,priority queue time,copy&free graph time,dfs time" << endl;
    }

    Simulator simulator(adg);
    simulator.simulate_wdelay(p, d, outFile, timeout, term_shortcut);
    outFile.close();
  }
  return 0;
}