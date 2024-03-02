#include <fstream>
#include <boost/program_options.hpp>
#include "Algorithm/Astar.h"

// print the state in which replanning (Switchable ADG optimization) happens.
// The state include the current locations and target locations of all agents.
void print_for_replanning(ADG &adg, vector<int> states, ofstream &outFile_path) {
  outFile_path << "version 1\n";
  int agentCnt = get_agentCnt(adg);
  for (int agent = 0; agent < agentCnt; agent ++) {
    outFile_path << "4	random-32-32-10.map	32	32	";
    Location l = get_state_target(adg, agent, states[agent]);
    Location e = get_state_target(adg, agent, get_stateCnt(adg, agent)-1);
    outFile_path << get<1>(l) << "	" << get<0>(l) << "	" << get<1>(e) << "	" << get<0>(e) << "	18.72792206\n";
  }
}

int Simulator::step_wdelay(int p, bool *delay_mark, vector<int> &delayed_agents) {
  int agentCnt = get_agentCnt(adg);

  random_device rd;  
  mt19937 gen(rd());
  uniform_int_distribution<> distrib(1, 1000);

  // if dependencies are satisified, then an agent is movable
  vector<int> movable(agentCnt, 0);
  // if an agent arrives its goal, then it has stopped.
  vector<int> haventStop(agentCnt, 0);
  int timeSpent = checkMovable(movable, haventStop);
  int moveCnt = 0;

  for (int agent = 0; agent < agentCnt; agent++) {
    if (haventStop[agent] == 1) {
      // each agent is delayed independently
      if (distrib(gen) <= p) {
        delayed_agents[agent] = 1;
        *delay_mark = true;
      }
    }
  }

  // if delayed
  if (*delay_mark) {
    return 0;
  }

  for (int agent = 0; agent < agentCnt; agent++) {
    if (movable[agent] == 1) {
      // an agent moves to its next state if moveable
      states[agent] += 1;
      moveCnt += 1;
    }
  }
  // if no agent can move but still cost time (namely, some agents haven't arrive their goals)
  // this means a global deadlock happens.
  if (moveCnt == 0 && timeSpent != 0) {
    return -1;
  }
  // return total time cost in this timestep.
  return timeSpent;
}

int Simulator::simulate_wdelay(int p, int dlow, int dhigh, ofstream &outFile, ofstream &outFile_slow, ofstream &outFile_path, ofstream &outFile_setup, const char * outFileName_Execution, int timeout) {
  int agentCnt = get_agentCnt(adg);
  int stepSpend = 1;
  bool delay_mark = false;
  vector<int> delayed_agents(agentCnt, 0);

  while (stepSpend != 0) {
    stepSpend = step_wdelay(p, &delay_mark, delayed_agents);
    // if a delay just happened
    // NOTE(rivers): multiple agents can be delayed independently at the same timestep.
    if (delay_mark)
    {
      print_for_replanning(adg, states, outFile_path);
      outFile_path.close();
      // int delayed_state = states[delayed_agent];

      // switchable edge count
      int input_sw_cnt;

      // construct the delayed ADG by inserting dummy nodes
      microseconds timer_constructADG(0);
      auto start = high_resolution_clock::now();
      ADG adg_delayed = construct_delayed_ADG(adg, dlow, dhigh, delayed_agents, states, &input_sw_cnt, outFile_setup);
      outFile_setup.close();
      auto stop = high_resolution_clock::now();
      timer_constructADG += duration_cast<microseconds>(stop - start);

      ADG adg_delayed_copy = copy_ADG(adg_delayed);
      set_switchable_nonSwitchable(get<0>(adg_delayed_copy));
      // simulate from the initial state
      Simulator simulator_original(adg_delayed_copy);
      int originalTime = simulator_original.print_soln();
      // simulate from the current state
      Simulator simulator_ori_trunc(adg_delayed_copy, states);
      int oriTime_trunc = simulator_ori_trunc.print_soln();

      // make a copy of delayed ADG for slow search later
      ADG adg_delayed_slow = copy_ADG(adg_delayed);

      /* [start] Optimize Switchable ADG use graph-based search */
      microseconds timer(0);
      start = high_resolution_clock::now();
      Astar search(timeout, true);
      ADG replanned_adg = search.startExplore(adg_delayed, input_sw_cnt);
      stop = high_resolution_clock::now();
      timer += duration_cast<microseconds>(stop - start);

      if (duration_cast<seconds>(timer).count() >= timeout) {
        outFile << timer.count() << "," << 
        timer.count() + timer_constructADG.count() << ",,,,,";
        search.print_stats(outFile);
        exit(0);
      }
      
      Simulator simulator_res(replanned_adg);
      int timeSum = simulator_res.print_soln(outFileName_Execution);
      Simulator simulator_res_trunc(replanned_adg, states);
      int timeSum_trunc = simulator_res_trunc.print_soln();

      outFile << timer.count() << "," << 
      timer.count() + timer_constructADG.count() << "," << 
      originalTime << "," <<
      timeSum << "," << 
      oriTime_trunc << "," <<
      timeSum_trunc << ",";
      
      search.print_stats(outFile);
      /* [ end ] Optimize Switchable ADG use graph-based search */

      /* [start] Optimize Switchable ADG use simulation-based search */
      microseconds timer_slow(0);
      start = high_resolution_clock::now();
      Astar search_slow(timeout, false);
      ADG replanned_slow = search_slow.startExplore(adg_delayed_slow, input_sw_cnt);
      stop = high_resolution_clock::now();
      timer_slow += duration_cast<microseconds>(stop - start);

      if (duration_cast<seconds>(timer_slow).count() >= timeout) {
        outFile_slow << timer_slow.count() << "," << 
        timer_slow.count() + timer_constructADG.count() << ",,,,,";
        search_slow.print_stats(outFile_slow);
        exit(0);
      }
      
      Simulator simulator_slow(replanned_slow);
      timeSum = simulator_slow.print_soln();
      Simulator simulator_slow_trunc(replanned_slow, states);
      timeSum_trunc = simulator_slow_trunc.print_soln();

      outFile_slow << timer_slow.count() << "," << 
      timer_slow.count() + timer_constructADG.count() << "," << 
      originalTime << "," <<
      timeSum << "," << 
      oriTime_trunc << "," <<
      timeSum_trunc << ",";
      
      search_slow.print_stats(outFile_slow);
      exit(0);
      /* [ end ] Optimize Switchable ADG use simulation-based search */

    }
    if (stepSpend < 0) return -1; // stuck
  }
  std::cout << "no delay happened\n";
  return 0;
}

int main(int argc, char** argv) {

  namespace po = boost::program_options;
  po::options_description desc("Switch ADG Optimization");
  desc.add_options()
    ("help", "show help message")
    ("path_file,p",po::value<std::string>()->required(),"path file to construct ADG")
    ("delay_prob,d",po::value<int>()->default_value(10),"delay probability*1000. need to be an integer")
    ("delay_steps_low,l",po::value<int>()->default_value(10),"the lowerbound of delay steps")
    ("delay_steps_high,h",po::value<int>()->default_value(20),"the upperbound of delay steps")
    ("time_limit,t",po::value<int>()->required(),"time limit in seconds. need to be an integer")
    ("graph_based_ofp,g",po::value<std::string>()->required(),"the output file path of graph-based search")
    ("simulation_based_ofp,s",po::value<std::string>()->required(),"the output file path of simulation-based search")
    ("location_ofp,c",po::value<std::string>()->required(),"the output file path of locations")
    ("delay_setup_ofp,r",po::value<std::string>()->required(),"the output file path of delay setups")
    ("execution_ofp,e",po::value<std::string>()->required(),"the output file path of execution")
  ;

  po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}
  
	po::notify(vm);

  // mapf path file
  const char* fileName = vm["path_file"].as<std::string>().c_str();
  // delay probablity
  int p = vm["delay_prob"].as<int>();
  // lower bound of delay length
  int dlow = vm["delay_steps_low"].as<int>();
  // upper bound of delay length
  int dhigh = vm["delay_steps_high"].as<int>();
  // time limit
  int timeout = vm["time_limit"].as<int>();

  // stats for graph-based module
  const char* outFileName = vm["graph_based_ofp"].as<std::string>().c_str();
  // stats for execution-based module
  const char* outFileName_slow = vm["simulation_based_ofp"].as<std::string>().c_str();
  // output file for the start and goal locations when a delay happens
  const char* outFileName_path = vm["location_ofp"].as<std::string>().c_str();
  // ? output file for the index of the delayed agents and the length of the delay
  const char* outFileName_setup = vm["delay_setup_ofp"].as<std::string>().c_str();
  // output file for the new execution after delay
  const char* outFileName_execution = vm["execution_ofp"].as<std::string>().c_str();

  // construct ADG from paths
  ADG adg = construct_ADG(fileName);

  ofstream outFile;
  outFile.open(outFileName, ios::app);
  ofstream outFile_slow;
  outFile_slow.open(outFileName_slow, ios::app);
  ofstream outFile_path;
  outFile_path.open(outFileName_path, ios::app);
  ofstream outFile_setup;
  outFile_setup.open(outFileName_setup, ios::app);
  if (outFile.is_open()) {
    // if (print_header) {
    //   outFile << "search time,search + construct time,original total cost,"	
    //   << "solution total cost,original remain cost,solution remain cost,"
    //   << "explored node,pruned node,added node,heuristic time,branch time,"
    //   << "sort time,priority queue time,copy&free graph time,dfs time" << endl;
    // }

    Simulator simulator(adg);
    simulator.simulate_wdelay(p, dlow, dhigh, outFile, outFile_slow, outFile_path, outFile_setup, outFileName_execution, timeout);
    outFile.close();
    outFile_slow.close();
  }
  return 0;
}