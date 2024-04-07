#include <fstream>
#include <boost/program_options.hpp>
#include "Algorithm/Astar.h"
#include "nlohmann/json.hpp"

using json=nlohmann::json;

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

int Simulator::step_wdelay(int p, bool &delay_mark, vector<int> &delayed_agents) {
  int agentCnt = get_agentCnt(adg);

  random_device rd;  
  mt19937 gen(rd());
  uniform_int_distribution<> distrib(1, 1000);

  // if dependencies are satisified, then an agent is movable
  vector<int> movable(agentCnt, 0);
  // if an agent arrives its goal, then it has stopped.
  vector<int> haventStop(agentCnt, 0);
  int timeSpent = checkMovable(movable, haventStop, true);
  int moveCnt = 0;

  for (int agent = 0; agent < agentCnt; agent++) {
    if (haventStop[agent] == 1) {
      // each agent is delayed independently
      if (distrib(gen) <= p) {
        delayed_agents[agent] = 1;
        delay_mark = true;
      }
    }
  }

  // if delayed
  if (delay_mark) {
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
    stepSpend = step_wdelay(p, delay_mark, delayed_agents);
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
      ADG adg_delayed = construct_delayed_ADG(adg, dlow, dhigh, delayed_agents, states, input_sw_cnt, outFile_setup);
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
      ADG replanned_adg = search.startExplore(adg_delayed, originalTime, input_sw_cnt, states);
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
      ADG replanned_slow = search_slow.startExplore(adg_delayed_slow, originalTime, input_sw_cnt, states);
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

void simulate(
  const string & path_fp, 
  const string & sit_fp, 
  int time_limit, 
  const string & algo, 
  const string & branch_order,
  bool use_grouping,
  const string & heuristic,
  bool early_termination,
  double w_astar,
  double w_focal,
  uint random_seed,
  const string & stat_ofp, 
  const string & new_path_ofp
) {
  ADG adg=construct_ADG(path_fp.c_str());
  ifstream in(sit_fp);
  json data=json::parse(in);

  size_t agent_num=get_agentCnt(adg);

  vector<int> states=data.at("states").get<vector<int> >();
  vector<int> delay_steps=data.at("delay_steps").get<vector<int> >();

  // TODO(rivers): maybe check path_fp with the path_fp saved in sit_fp as well.
  if (states.size()!=agent_num || delay_steps.size()!=agent_num) {
    std::cout<<"size mismatch: "<<states.size()<<" "<<delay_steps.size()<<" "<<agent_num<<std::endl;
    exit(50);
  }

  // switchable edge count
  int input_sw_cnt;

  // construct the delayed ADG by inserting dummy nodes
  microseconds timer_constructADG(0);
  auto start = high_resolution_clock::now();
  ADG adg_delayed = construct_delayed_ADG(adg, delay_steps, states, input_sw_cnt);
  auto stop = high_resolution_clock::now();
  timer_constructADG += duration_cast<microseconds>(stop - start);

  // simulate without replanning
  ADG adg_delayed_copy = copy_ADG(adg_delayed);
  set_switchable_nonSwitchable(get<0>(adg_delayed_copy));
  // simulate from the initial state
  Simulator simulator_original(adg_delayed_copy);
  int originalTime = simulator_original.print_soln();
  // simulate from the current state
  Simulator simulator_ori_trunc(adg_delayed_copy, states);
  int oriTime_trunc = simulator_ori_trunc.print_soln();

  // replanning ADG
  shared_ptr<Astar> search=make_shared<Astar>(
    time_limit, 
    true, 
    branch_order,  
    use_grouping, 
    heuristic, 
    early_termination, 
    w_astar,
    w_focal,
    random_seed
  );

  json stats;

  // basic information
  stats["algo"]=algo;
  stats["branch_order"]=branch_order;
  stats["use_grouping"]=use_grouping;
  stats["heuristic"]=heuristic;
  stats["early_termination"]=early_termination;
  stats["w_astar"]=w_astar;
  stats["w_focal"]=w_focal;
  stats["random_seed"]=random_seed;
  stats["time_limit"]=time_limit*1000000; // in micro-seconds
  stats["path_fp"]=path_fp;
  stats["sit_fp"]=sit_fp;

  // default status
  stats["status"]="Fail";
  stats["search_time"]=time_limit*1000000; // in micro-seconds
  stats["total_time"]=time_limit*1000000; // in micro-seconds
  stats["ori_total_cost"]=originalTime;
  stats["ori_trunc_cost"]=oriTime_trunc;
  // implication: fail to replan, just use the original one
  stats["total_cost"]=originalTime;
  stats["trunc_cost"]=oriTime_trunc;
  stats["explored_node"]=nullptr;
  stats["pruned_node"]=nullptr;
  stats["added_node"]=nullptr;
  stats["vertex"]=nullptr;
  stats["sw_edge"]=nullptr;
  stats["extra_heuristic_time"]=nullptr;
  stats["heuristic_time"]=nullptr;
  stats["branch_time"]=nullptr;
  stats["sort_time"]=nullptr;
  stats["priority_queue_time"]=nullptr;
  stats["copy_free_graphs_time"]=nullptr;
  stats["termination_time"]=nullptr;
  stats["dfs_time"]=nullptr;
  stats["grouping_time"]=nullptr;
  stats["group"]=nullptr;
  stats["group_merge_edge"]=nullptr;
  stats["group_size_max"]=nullptr;
  stats["group_size_min"]=nullptr;
  stats["group_size_avg"]=nullptr;
  
  std::ofstream out(stat_ofp);
  out<<stats.dump(4)<<std::endl;
  out.close();

  microseconds timer(0);
  start = high_resolution_clock::now();
  ADG replanned_adg = search->startExplore(adg_delayed, originalTime, input_sw_cnt, states);
  stop = high_resolution_clock::now();
  timer += duration_cast<microseconds>(stop - start);


  if (duration_cast<seconds>(timer).count() < time_limit) {
    // simulate with new paths
    Simulator simulator_res(replanned_adg);
    int timeSum = simulator_res.print_soln(new_path_ofp.c_str());
    Simulator simulator_res_trunc(replanned_adg, states);
    int timeSum_trunc = simulator_res_trunc.print_soln();

    stats["status"]="Succ";
    stats["search_time"]=timer.count();
    stats["total_time"]=timer.count() + timer_constructADG.count();
    stats["ori_total_cost"]=originalTime;
    stats["ori_trunc_cost"]=oriTime_trunc;
    stats["total_cost"]=timeSum;
    stats["trunc_cost"]=timeSum_trunc;
  } else {
    stats["status"]="Timeout";
    stats["search_time"]=time_limit*1000000; // in micro-seconds
    stats["total_time"]=time_limit*1000000; // in micro-seconds
    stats["ori_total_cost"]=originalTime;
    stats["ori_trunc_cost"]=oriTime_trunc;
    // implication: fail to replan, just use the original one
    stats["total_cost"]=originalTime;
    stats["trunc_cost"]=oriTime_trunc;
  }

  search->print_stats(stats);

  out.open(stat_ofp);
  out<<stats.dump(4)<<std::endl;
  out.close();
}

int main(int argc, char** argv) {

  namespace po = boost::program_options;
  po::options_description desc("Switch ADG Optimization");
  desc.add_options()
    ("help", "show help message")
    ("path_fp,p",po::value<std::string>()->required(),"path file to construct ADG")
    ("sit_fp,s",po::value<std::string>()->required(),"situation file to construct delayed ADG")
    ("time_limit,t",po::value<int>()->required(),"time limit in seconds. need to be an integer")
    ("algo,a",po::value<std::string>()->required(),"replaning algorithm to use, [graph]")
    ("stat_ofp,o",po::value<std::string>()->required(),"the output file path of statistics")
    ("new_path_ofp,n",po::value<std::string>()->required(),"the output file path of new paths")
    ("branch_order,b",po::value<std::string>()->required(),"the branch order to use, [default, conflict, largest_diff, random, earliest]")
    ("use_grouping,g",po::value<bool>()->required(),"whether to use grouping")
    ("heuristic,h",po::value<std::string>()->required(),"the heuristic to use, [zero, cg_greedy]")
    ("early_termination,e",po::value<bool>()->required(),"whether to use early termination")
    ("w_astar",po::value<double>()->default_value(1.0),"heuristic weight for weighted A Star")
    ("w_focal",po::value<double>()->default_value(1.0),"heuristic weight for focal search")
    ("random_seed,r",po::value<uint>()->default_value(0),"random seed")
  ;

  po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}
  
	po::notify(vm);

  string path_fp=vm.at("path_fp").as<string>();
  string sit_fp=vm.at("sit_fp").as<string>();
  int time_limit=vm.at("time_limit").as<int>();
  string algo=vm.at("algo").as<string>();
  string branch_order=vm.at("branch_order").as<string>();
  uint random_seed=vm.at("random_seed").as<uint>();
  string stat_ofp=vm.at("stat_ofp").as<string>();
  string new_path_ofp=vm.at("new_path_ofp").as<string>();
  bool use_grouping=vm.at("use_grouping").as<bool>();
  string heuristic=vm.at("heuristic").as<string>();
  double w_astar=vm.at("w_astar").as<double>();
  double w_focal=vm.at("w_focal").as<double>();

  if (w_astar>1.0 && w_focal>1.0) {
    std::cout<<"Using both weighted astar and focal search is not supported."<<std::endl;
    exit(-1);
  }

  bool early_termination=vm.at("early_termination").as<bool>();

  simulate(
    path_fp,
    sit_fp,
    time_limit,
    algo,
    branch_order,
    use_grouping,
    heuristic,
    early_termination,
    w_astar,
    w_focal,
    random_seed,
    stat_ofp,
    new_path_ofp
  );

  return 0;
}