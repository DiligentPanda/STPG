#include <fstream>
#include <boost/program_options.hpp>
#include "Algorithm/Astar.h"
#include "nlohmann/json.hpp"

using json=nlohmann::json;

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
  // simulate from the initial state
  Simulator simulator_original(adg_delayed_copy);
  int originalTime = simulator_original.print_soln();
  // simulate from the current state
  Simulator simulator_ori_trunc(adg_delayed_copy, states);
  int oriTime_trunc = simulator_ori_trunc.print_soln();

  cout<<"original_total_cost: "<<originalTime<<", original_trunc_cost: "<<oriTime_trunc<<endl;

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
  auto replanned_adg = search->startExplore(adg_delayed, originalTime, states);
  stop = high_resolution_clock::now();
  timer += duration_cast<microseconds>(stop - start);


  if (duration_cast<seconds>(timer).count() < time_limit) {
    // simulate with new paths
    Simulator simulator_res(replanned_adg);
    int timeSum = simulator_res.print_soln(new_path_ofp.c_str());
    Simulator simulator_res_trunc(replanned_adg, states);
    int timeSum_trunc = simulator_res_trunc.print_soln();

    cout<<"optimized_total_cost: "<<timeSum<<", optimized_trunc_cost: "<<timeSum_trunc<<endl;

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