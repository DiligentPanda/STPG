#include <fstream>
#include <boost/program_options.hpp>
#include "Algorithm/Astar.h"
#include "nlohmann/json.hpp"
#include "MILP/milp_solver.h"
#include "solver.h"

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
  double w_astar,
  double w_focal,
  uint random_seed,
  const string & stat_ofp, 
  const string & new_path_ofp
) {
  auto adg=construct_ADG(path_fp.c_str());
  ifstream in(sit_fp);
  json data=json::parse(in);

  size_t agent_num=adg->get_num_agents();

  vector<int> states=data.at("states").get<vector<int> >();
  vector<int> delay_steps=data.at("delay_steps").get<vector<int> >();

  // TODO(rivers): maybe check path_fp with the path_fp saved in sit_fp as well.
  if (states.size()!=agent_num || delay_steps.size()!=agent_num) {
    std::cout<<"size mismatch: "<<states.size()<<" "<<delay_steps.size()<<" "<<agent_num<<std::endl;
    exit(50);
  }

  // construct the delayed ADG by inserting dummy nodes
  microseconds timer_constructADG(0);
  auto start = high_resolution_clock::now();
  auto adg_delayed = construct_delayed_ADG(adg, delay_steps, states);
  auto stop = high_resolution_clock::now();
  timer_constructADG += duration_cast<microseconds>(stop - start);

  // simulate without replanning
  auto adg_delayed_copy = adg_delayed->copy();
  adg_delayed_copy->fix_all_switchable_type2_edges();
  // simulate from the current state
  Simulator simulator_original(adg_delayed_copy, states);
  int original_cost = simulator_original.print_soln();

  cout<<"original cost: "<<original_cost<<endl;

  shared_ptr<Solver> solver;
  
  if (algo=="milp") {
    solver=make_shared<MILPSolver>(grouping_method, time_limit, 0.0);
  } else if (algo=="search") {
    solver=make_shared<Astar>(
      time_limit, 
      true, 
      branch_order,  
      grouping_method, 
      heuristic, 
      early_termination, 
      incremental,
      w_astar,
      w_focal,
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
  auto replanned_adg = solver->solve(adg_delayed, original_cost, states);
  stop = high_resolution_clock::now();
  timer += duration_cast<microseconds>(stop - start);


  if (duration_cast<seconds>(timer).count() < time_limit) {
    // simulate with the replanned ADG
    Simulator simulator(replanned_adg, states);
    int cost= simulator.print_soln();

    cout<<"optimized cost: "<<cost<<endl;

    stats["status"]="Succ";
    // stats["search_time"]=timer.count();
    stats["total_time"]=timer.count() + timer_constructADG.count();
    stats["cost"]=cost;
  } else {
    stats["status"]="Timeout";
    // stats["search_time"]=time_limit*1000000; // in micro-seconds
    stats["total_time"]=time_limit*1000000; // in micro-seconds
    // implication: fail to replan, just use the original one
    stats["cost"]=original_cost;
  }

  solver->write_stats(stats);

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
    ("algo,a",po::value<std::string>()->required(),"replaning algorithm to use, [search, milp]")
    ("stat_ofp,o",po::value<std::string>()->required(),"the output file path of statistics")
    ("new_path_ofp,n",po::value<std::string>()->required(),"the output file path of new paths")
    ("branch_order,b",po::value<std::string>()->required(),"the branch order to use, [default, conflict, largest_diff, random, earliest]")
    ("grouping_method,g",po::value<std::string>()->required(),"the grouping method to use, [none, simple, simple_merge, all]")
    ("heuristic,h",po::value<std::string>()->required(),"the heuristic to use, [zero, cg_greedy, wcg_greedy, fast_wcg_greedy]")
    ("early_termination,e",po::value<bool>()->required(),"whether to use early termination")
    ("incremental,i",po::value<bool>()->required(),"whether to use incremental update")
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
  string grouping_method=vm.at("grouping_method").as<string>();
  string heuristic=vm.at("heuristic").as<string>();
  double w_astar=vm.at("w_astar").as<double>();
  double w_focal=vm.at("w_focal").as<double>();

  if (w_astar>1.0 && w_focal>1.0) {
    std::cout<<"Using both weighted astar and focal search is not supported."<<std::endl;
    exit(-1);
  }

  bool early_termination=vm.at("early_termination").as<bool>();
  bool incremental=vm.at("incremental").as<bool>();

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
    random_seed,
    stat_ofp,
    new_path_ofp
  );

  return 0;
}