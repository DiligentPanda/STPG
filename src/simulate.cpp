#include <fstream>
#include <boost/program_options.hpp>
#include "Algorithm/Astar.h"
#include "nlohmann/json.hpp"
#include "MILP/milp_solver.h"
#include "solver.h"
#include "graph/generate_graph.h"
#include "define.h"
#include "simulation/new_simulator.h"

using json=nlohmann::json;

void simulate(
  const string & path_fp, 
  const string & sit_fp, 
  int time_limit, 
  const string & algo, 
  const string & branch_order,
  const string & grouping_method,
  const string & heuristic,
  bool early_termination,
  bool incremental,
  COST_TYPE w_astar,
  COST_TYPE w_focal,
  int horizon,
  uint random_seed,
  const string & stat_ofp, 
  const string & new_path_ofp
) {
  auto graph=construct_graph(path_fp.c_str());
  ifstream in(sit_fp);
  json data=json::parse(in);

  size_t agent_num=graph->get_num_agents();

  vector<int> states=data.at("states").get<vector<int> >();
  vector<int> delay_steps=data.at("delay_steps").get<vector<int> >();
  vector<COST_TYPE> delay_steps_cost(delay_steps.begin(),delay_steps.end());
  // NOTE: we must call this function to prune the edges before the current states
  // otherwise search algorithm may consider edges before the current states and cause errors.

  // TODO(rivers): maybe check path_fp with the path_fp saved in sit_fp as well.
  if (states.size()!=agent_num || delay_steps.size()!=agent_num) {
    std::cout<<"size mismatch: "<<states.size()<<" "<<delay_steps.size()<<" "<<agent_num<<std::endl;
    exit(50);
  }

  std::shared_ptr<GroupManager> group_manager(nullptr);
  std::chrono::microseconds grouping_time(0);
  std::cout<<"grouping method: "<<grouping_method<<std::endl;
  if (grouping_method!="none") {
    auto switchable_graph = graph->copy();
    switchable_graph->make_switchable();
    // std::cout<<"input_sw_cnt: "<<input_sw_cnt<<std::endl;
    auto start = high_resolution_clock::now();
    group_manager=std::make_shared<GroupManager>(switchable_graph, *(switchable_graph->curr_states), grouping_method);
    auto end = high_resolution_clock::now();
    grouping_time = duration_cast<microseconds>(end - start);
    std::cout<<"group size: "<<group_manager->groups.size()<<std::endl;
   // group_manager->print_groups();
  }

  int total_delays=0;
  for (auto delay_step:delay_steps) {
    total_delays+=delay_step;
  }
  std::cout<<"total delays: "<<total_delays<<std::endl;

  // update states and set delays
  microseconds timer_construct_graph(0);
  auto start = high_resolution_clock::now();
  graph->update_curr_states(states);
  graph->delay(delay_steps_cost);
  std::cout<<"#sw: "<<graph->get_num_switchable_edges()<<", #nsw: "<<graph->get_num_non_switchable_edges()<<", #states: "<<graph->get_num_states()<<std::endl;
  auto stop = high_resolution_clock::now();
  timer_construct_graph += duration_cast<microseconds>(stop - start);

  auto new_simulator=make_shared<NewSimulator>();

  // simulate without replanning from current states
  int original_cost = new_simulator->simulate(graph);
  cout<<"original cost: "<<original_cost<<endl;

  shared_ptr<Solver> solver;
  
  if (algo=="milp") {
    solver=make_shared<MILPSolver>(time_limit, horizon, group_manager, 0.0);
  } else if (algo=="search") {
    solver=make_shared<Astar>(
      time_limit, 
      true, 
      branch_order,  
      heuristic, 
      early_termination, 
      incremental,
      w_astar,
      w_focal,
      horizon,
      group_manager,
      random_seed
    );
  } else {
    std::cout<<"Unsupported algorithm: "<<algo<<std::endl;
    exit(-1);
  }

  json stats;

  // basic information
  stats["algo"]=algo;
  stats["branch_order"]=branch_order;
  stats["grouping_method"]=grouping_method;
  stats["heuristic"]=heuristic;
  stats["early_termination"]=early_termination;
  stats["incremental"]=incremental;
  stats["w_astar"]=w_astar;
  stats["w_focal"]=w_focal;
  stats["horizon"]=horizon;
  stats["random_seed"]=random_seed;
  stats["time_limit"]=time_limit*1000000; // in micro-seconds
  stats["path_fp"]=path_fp;
  stats["sit_fp"]=sit_fp;

  // default status
  stats["status"]="Fail";
  stats["search_time"]=time_limit*1000000; // in micro-seconds
  stats["total_time"]=time_limit*1000000; // in micro-seconds
  stats["original_cost"]=original_cost;
  // implication: fail to replan, just use the original one
  stats["cost"]=original_cost;
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
  stats["grouping_time"]=grouping_time.count();
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
  auto replanned_graph = solver->solve(graph);
  stop = high_resolution_clock::now();
  timer += duration_cast<microseconds>(stop - start);

  solver->write_stats(stats);

  if (duration_cast<seconds>(timer).count() < time_limit) {
    // simulate with the replanned graph
    int cost = new_simulator->simulate(replanned_graph);
    cout<<"optimized cost: "<<cost<<endl;

    stats["status"]="Succ";
    // stats["search_time"]=timer.count();
    stats["total_time"]=timer.count() + timer_construct_graph.count();
    stats["cost"]=cost;
  } else {
    stats["status"]="Timeout";
    // stats["search_time"]=time_limit*1000000; // in micro-seconds
    stats["total_time"]=time_limit*1000000; // in micro-seconds
    // implication: fail to replan, just use the original one
    stats["cost"]=original_cost;
  }


  std::cout<<"search time: "<<stats["search_time"].get<float>()/1000000.0<<", total time: "<<stats["total_time"].get<float>()/1000000.0<<std::endl;
  std::cout<<"construct graph time: "<<timer_construct_graph.count()/1000000.0<<std::endl;

  out.open(stat_ofp);
  out<<stats.dump(4)<<std::endl;
  out.close();
}


void full_simulate(
  const string & path_fp,
  float delay_prob,
  int delay_steps_low,
  int delay_steps_high,
  int time_limit, 
  const string & algo, 
  const string & branch_order,
  const string & grouping_method,
  const string & heuristic,
  bool early_termination,
  bool incremental,
  COST_TYPE w_astar,
  COST_TYPE w_focal,
  int horizon,
  uint random_seed,
  const string & stat_ofp, 
  const string & new_path_ofp
) {
  auto graph=construct_graph(path_fp.c_str());

  std::shared_ptr<GroupManager> group_manager(nullptr);
  std::chrono::microseconds grouping_time(0);
  std::cout<<"grouping method: "<<grouping_method<<std::endl;
  if (grouping_method!="none") {
    auto switchable_graph = graph->copy();
    switchable_graph->make_switchable();
    // std::cout<<"input_sw_cnt: "<<input_sw_cnt<<std::endl;
    auto start = high_resolution_clock::now();
    group_manager=std::make_shared<GroupManager>(switchable_graph, *(switchable_graph->curr_states), grouping_method);
    auto end = high_resolution_clock::now();
    grouping_time = duration_cast<microseconds>(end - start);
    std::cout<<"group size: "<<group_manager->groups.size()<<std::endl;
  }

  shared_ptr<Solver> solver;
  
  if (algo=="milp") {
    solver=make_shared<MILPSolver>(time_limit, horizon, group_manager, 0.0);
  } else if (algo=="search") {
    solver=make_shared<Astar>(
      time_limit, 
      true, 
      branch_order,  
      heuristic, 
      early_termination, 
      incremental,
      w_astar,
      w_focal,
      horizon,
      group_manager,
      random_seed
    );
  } else if (algo=="none") {

  } else {
    std::cout<<"Unsupported algorithm: "<<algo<<std::endl;
    exit(-1);
  }

  auto no_delay_simulator=make_shared<NewSimulator>();
  // simulate without replanning from current states
  int original_cost = no_delay_simulator->simulate(graph);

  std::cout<<"original cost: "<<original_cost<<std::endl;

  // when we fix the random seed and use the uniform state delay, the simulated delays will be the same
  auto delay_simulator=make_shared<NewSimulator>(solver,delay_prob,delay_steps_low,delay_steps_high,random_seed,1);
  // simulate without replanning from current states
  int cost_after_delay_with_stpg = delay_simulator->simulate(graph);
  int ideal_cost_after_delay=original_cost+delay_simulator->get_total_delays();

  auto delay_simulator2=make_shared<NewSimulator>(nullptr,delay_prob,delay_steps_low,delay_steps_high,random_seed,1);
  int cost_after_delay = delay_simulator2->simulate(graph);
  if (delay_simulator2->get_total_delays()!=delay_simulator->get_total_delays()) {
    std::cout<<"total delays mismatch: "<<delay_simulator2->get_total_delays()<<" "<<delay_simulator->get_total_delays()<<std::endl;
    exit(-1);
  }

  std::cout<<"cost after delay with stpg: "<<cost_after_delay_with_stpg<<std::endl;
  std::cout<<"cost after delay: "<<cost_after_delay<<std::endl;
  std::cout<<"ideal cost after delay: "<<ideal_cost_after_delay<<std::endl;

  float improvement=(float)(cost_after_delay-cost_after_delay_with_stpg)/(float)(cost_after_delay);

  std::cout<<"improvement: "<<improvement<<std::endl;
}

int main(int argc, char** argv) {

  namespace po = boost::program_options;
  po::options_description desc("Switch graph Optimization");
  desc.add_options()
    ("help", "show help message")
    ("path_fp,p",po::value<std::string>()->required(),"path file to construct graph")
    ("sit_fp,s",po::value<std::string>()->default_value(""),"situation file to construct delayed graph")
    ("time_limit,t",po::value<int>()->required(),"time limit in seconds. need to be an integer")
    ("algo,a",po::value<std::string>()->required(),"replaning algorithm to use, [search, milp, none]")
    ("stat_ofp,o",po::value<std::string>()->required(),"the output file path of statistics")
    ("new_path_ofp,n",po::value<std::string>()->required(),"the output file path of new paths")
    ("branch_order,b",po::value<std::string>()->required(),"the branch order to use, [default, conflict, largest_diff, random, earliest]")
    ("grouping_method,g",po::value<std::string>()->required(),"the grouping method to use, [none, simple, simple_merge, all]")
    ("heuristic,h",po::value<std::string>()->required(),"the heuristic to use, [zero, cg_greedy, wcg_greedy, fast_wcg_greedy]")
    ("early_termination,e",po::value<bool>()->required(),"whether to use early termination")
    ("incremental,i",po::value<bool>()->required(),"whether to use incremental update")
    ("w_astar",po::value<COST_TYPE>()->default_value(1.0),"heuristic weight for weighted A Star")
    ("w_focal",po::value<COST_TYPE>()->default_value(1.0),"heuristic weight for focal search")
    ("random_seed,r",po::value<uint>()->default_value(10),"random seed")
    ("delay_prob",po::value<float>()->default_value(0.03),"delay probability (0<=p<=1)")
    ("delay_steps_low",po::value<int>()->default_value(10),"the lowerbound of delay steps")
    ("delay_steps_high",po::value<int>()->default_value(30),"the upperbound of delay steps")
    ("horizon",po::value<int>()->default_value(-1),"the horizon for swtichable TPG optimization, -1 means no limit")
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
  string grouping_method=vm.at("grouping_method").as<string>();
  string heuristic=vm.at("heuristic").as<string>();
  COST_TYPE w_astar=vm.at("w_astar").as<COST_TYPE>();
  COST_TYPE w_focal=vm.at("w_focal").as<COST_TYPE>();
  int horizon=vm.at("horizon").as<int>();


  // used for full simulation
  float delay_prob=vm.at("delay_prob").as<float>();
  int delay_steps_low=vm.at("delay_steps_low").as<int>();
  int delay_steps_high=vm.at("delay_steps_high").as<int>();


  if (w_astar>1.0 && w_focal>1.0) {
    std::cout<<"Using both weighted astar and focal search is not supported."<<std::endl;
    exit(-1);
  }

  bool early_termination=vm.at("early_termination").as<bool>();
  bool incremental=vm.at("incremental").as<bool>();


  if (sit_fp!="") {
    std::cout<<"single scenario simulation"<<std::endl;
    simulate(
      path_fp,
      sit_fp,
      time_limit,
      algo,
      branch_order,
      grouping_method,
      heuristic,
      early_termination,
      incremental,
      w_astar,
      w_focal,
      horizon,
      random_seed,
      stat_ofp,
      new_path_ofp
    );
  } else {
    std::cout<<"full simulation"<<std::endl;
    full_simulate(
      path_fp,
      delay_prob,
      delay_steps_low,
      delay_steps_high,
      time_limit,
      algo,
      branch_order,
      grouping_method,
      heuristic,
      early_termination,
      incremental,
      w_astar,
      w_focal,
      horizon,
      random_seed,
      stat_ofp,
      new_path_ofp
    );
  }

  return 0;
}