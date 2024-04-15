#include <fstream>
#include <boost/program_options.hpp>
#include "Algorithm/Astar.h"
#include <boost/filesystem.hpp>
#include <vector>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

bool load_situation_file(const string & situation_fp, Simulator & simulator, std::vector<int> & delay_steps_vec) {
  ifstream in(situation_fp);
  if (!in) {
    return false;
  }

  json data=json::parse(in);
  simulator.states=data.at("states").get<std::vector<int> >();
  delay_steps_vec=data.at("delay_steps").get<std::vector<int> >();
  return true;
}

bool save_situation_file(const string & situation_fp, const string & path_fp, const Simulator & simulator, const std::vector<int> & delay_steps_vec) {
  json data;
  data["path_file"]=path_fp;
  data["states"]=simulator.states;
  data["delay_steps"]=delay_steps_vec;
  
  ofstream out(situation_fp);
  if (out) {
    out<<data.dump(4)<<std::endl;
  } else {
    return false;
  }
  return true;
}

void gen_random_situation(int idx, const string & path_fp, uint seed, int delay_prob, int delay_steps_low, int delay_steps_high, const string & situation_fp) {
  auto adg=construct_ADG(path_fp.c_str());
  Simulator simulator(adg);
  int num_agents=adg->get_num_agents();

  std::mt19937 rng(seed);
  std::uniform_int_distribution<int> delay_distrib(1,1000);
  std::uniform_int_distribution<int> delay_steps_distrib(delay_steps_low,delay_steps_high);

  std::vector<int> delay_steps_vec(num_agents,0);

  auto & state_cnts=*(adg->state_cnts);

  int stepSpend = 1;
  bool delayed = false;
  while (stepSpend != 0) {
    for (int aid=0;aid<num_agents;++aid) {
      // if arrived, don't consider delay
      if (simulator.states[aid]>=state_cnts[aid]-1) {
        continue;
      }
      if (delay_distrib(rng)<delay_prob) {
        int delay_steps=delay_steps_distrib(rng);
        delay_steps_vec[aid]=delay_steps;
        delayed=true;
      }
    }
    if (delayed) {
      bool succ=save_situation_file(situation_fp,path_fp,simulator,delay_steps_vec);
      if (!succ) {
        cout<<"Generating "+situation_fp<<" failed. Cannot open situation file."<<std::endl;
        exit(98);
      }
      break;
    }
    stepSpend = simulator.step(true);
    if (stepSpend < 0) {
      cout<<"Stuck in gen_random_situation(): "+situation_fp<<std::endl;
      exit(99);
    } // stuck
  }

  if (!delayed) {
    cout<<"no delay happens: "+situation_fp<<std::endl;
    exit(100);
  }
}

void gen_random_situations(const string & path_fp, int num, int delay_prob, int delay_steps_low, int delay_steps_high, const string & situation_fd) {
  const uint default_seed=std::random_device()()%1000000;
  const uint seed_step=97;

  boost::filesystem::path _situation_fd(situation_fd);
  boost::filesystem::create_directories(_situation_fd);

  boost::filesystem::path _path_fp(path_fp);
  auto file_name=_path_fp.stem();

  for (auto idx=0;idx<num;++idx) {
    // we will save it as a json
    auto _situation_fp=_situation_fd / (file_name.string()+"_sit_"+std::to_string(idx)+".json");
    string situation_fp=_situation_fp.string();
    uint seed=default_seed+seed_step*idx;
    gen_random_situation(idx, path_fp,seed,delay_prob, delay_steps_low, delay_steps_high, situation_fp);
  }
}

int main(int argc, char** argv) {

  namespace po = boost::program_options;
  po::options_description desc("Switch ADG Optimization");
  desc.add_options()
    ("help", "show help message")
    ("path_fp,p",po::value<std::string>()->required(),"path file to construct ADG")
    ("sit_num,n",po::value<int>()->required(),"the number of random situations for this instance")
    ("sit_fd,o",po::value<std::string>()->required(),"output folder to store situation files")
    ("delay_prob,d",po::value<int>()->default_value(10),"delay probability*1000. need to be an integer")
    ("delay_steps_low,l",po::value<int>()->default_value(10),"the lowerbound of delay steps")
    ("delay_steps_high,h",po::value<int>()->default_value(20),"the upperbound of delay steps")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  po::notify(vm);

  auto path_fp=vm.at("path_fp").as<string>();
  auto sit_num=vm.at("sit_num").as<int>();
  auto sit_fd=vm.at("sit_fd").as<string>();
  auto delay_prob=vm.at("delay_prob").as<int>();
  auto delay_steps_low=vm.at("delay_steps_low").as<int>();
  auto delya_steps_high=vm.at("delay_steps_high").as<int>();

  gen_random_situations(path_fp, sit_num, delay_prob, delay_steps_low, delya_steps_high, sit_fd);

  return 0;
}