/* Copyright (C) Jiaoyang Li
* Unauthorized copying of this file, via any medium is strictly prohibited
* Confidential
* Written by Jiaoyang Li <jiaoyanl@usc.edu>, Dec 2018
*/

/*driver.cpp
* Solve a MAPF instance on 2D grids.
*/

#include "constrained_map_loader.h"
#include "constrained_agents_loader.h"
#include "ICBSHSearchPairAnalysis.h"

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include<boost/tokenizer.hpp>
#include <chrono>

int main(int argc, char** argv) 
{
	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		// TODO: before search, we need to set map to the corresponding agent
		// we need the following now:
		// map_fp
		// path_fp
		// sit_fp
		("help", "produce help message")
		("map_fp", po::value<std::string>()->required(), "input file for map")
		("path_fp", po::value<std::string>()->required(), "input file for map")
		("sit_fp", po::value<std::string>()->required(), "input file for map")
		// ("agents,a", po::value<std::string>()->required(), "input file for agents")
		("output,o", po::value<std::string>()->required(), "output file for schedule")
		("solver,s", po::value<std::string>()->required(), "solvers (CBS, ICBS, CBSH, CBSH-CR, CBSH-R, CBSH-RM, CBSH-GR")
		// ("agentNum,k", po::value<int>()->default_value(0), "number of agents")
		("cutoffTime,t", po::value<float>()->default_value(7200), "cutoff time (seconds)")
		("seed,d", po::value<int>()->default_value(0), "random seed")
		("screen", po::value<int>()->default_value(0), "screen option (0: none; 1: results; 2:all)")
		("corridor", po::value<bool>()->default_value(false), "reason about 2-way branching corridor conflicts")
		("target", po::value<bool>()->default_value(false), "reason about target conflict")
		("parking", po::value<bool>()->default_value(false), "reason about target conflict")
		("kDelay", po::value<int>()->default_value(0), "Set max_k for k robust plan")
		("shrink", "shrink to hole on reaching target")
		("ignore-target",  "ignore all target conflict")
		("ignore-train", "ignore train conflict, act only robust cbs")
		("no-train-classify", "ignore train conflict, act only robust cbs")
		("lltp-only",  "only use lltp and occupation conflict find solution.")
		("diff-k",  "All agent have different k")
		// ("only_generate_instance", po::value<std::string>()->default_value(""),"no searching")
		("debug", "debug mode")
		("statistic","print statistic data")
		("pairAnalysis",po::value<int>(),"perform 2 agent analysis")
		("printFailedPair","print mdd and constraints for failed pair")
		("printPath", "print path to stdout")
		("writePath",po::value<std::string>()->default_value(""),"the path of a file to write paths")
		("exitOnNoSolution",po::value<bool>()->default_value(false),"if there is no solution, return -1")
		("f_w",po::value<float>()->default_value(1.0),"suboptimal factor>=1.0")

            ;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	po::notify(vm);

	int max_k = vm["kDelay"].as<int>();
	bool diff_k = vm.count("diff-k");

	if (vm["screen"].as<int>() == 2) {
		cout << "[CBS] Loading map and agents " << endl;
	}

	
	// read the map file and construct its two-dim array
	ConstrainedMapLoader* ml = new ConstrainedMapLoader(
		vm["map_fp"].as<string>(),
		vm["path_fp"].as<string>(),
		vm["sit_fp"].as<string>()
	);

	// read agents' start and goal locations
	ConstrainedAgentsLoader al(
		*ml,
		max_k,
		diff_k
	);

	if (vm["screen"].as<int>() == 2) {
		cout << "[CBS] Loading map and agents done! " << endl;
	}
	srand(vm["seed"].as<int>());

	options options1;

	options1.debug = vm.count("debug");
	options1.printFailedPair = vm.count("printFailedPair");
	options1.pairAnalysis = vm.count("pairAnalysis");
    options1.ignore_target = vm.count("ignore-target");
    options1.lltp_only = vm.count("lltp-only");
    options1.parking = vm["parking"].as<bool>();;
    options1.shrink = vm.count("shrink");
    options1.ignore_train = vm.count("ignore-train");
    options1.no_train_classify = vm.count("no-train-classify");

	// if (vm["only_generate_instance"].as<string>()!="") {
	// 	al.saveToFile(vm["only_generate_instance"].as<string>());
	// 	return 0;
	// }

	constraint_strategy s;
	if (vm["solver"].as<string>() == "ICBS")
		s = constraint_strategy::ICBS;
	else if (vm["solver"].as<string>() == "CBS")
		s = constraint_strategy::CBS;
	else if (vm["solver"].as<string>() == "CBSH")
		s = constraint_strategy::CBSH;
	else if (vm["solver"].as<string>() == "CBSH-RM")
		s = constraint_strategy::CBSH_RM;
	else
	{
		std::cout <<"WRONG SOLVER NAME!" << std::endl;
		return -1;
	}

	float f_w=vm["f_w"].as<float>();
	if (f_w<1.0) {
		std::cout<<"suboptimal factor should be >=1.0"<<std::endl;
		return -1;
	}

    ICBSSearchWithPairedAnalysis<ConstrainedMapLoader> icbs(ml, al, f_w, s, vm["cutoffTime"].as<float>() * CLOCKS_PER_SEC, vm["screen"].as<int>(), vm["kDelay"].as<int>(), options1);
	if (vm["solver"].as<string>() == "CBSH-RM")
	{
		icbs.rectangleMDD = true;
	}

	if (vm.count("corridor"))
	{
		icbs.corridor2 = vm["corridor"].as<bool>();
	}

	if (vm.count("target"))
	{
		icbs.targetReasoning = vm["target"].as<bool>();

	}

    if (vm.count("pairAnalysis")) {
        icbs.analysisEngine = new ICBSSearchWithPairedAnalysis<ConstrainedMapLoader>(&icbs, vm["pairAnalysis"].as<int>());
        icbs.analysisOutput = ofstream();
        icbs.analysisOutput.open(vm["output"].as<string>()+".FailedPairs",ios::trunc);
        icbs.analysisOutput<<"["<<endl;
    }
	
	bool res;
	res = icbs.runICBSSearch();

	if (!res && vm["exitOnNoSolution"].as<bool>()) {
		if (icbs.isTimeout()) {
			cout<<"Solution not found. Timeout: "<<vm["cutoffTime"].as<float>()<<" seconds"<<std::endl;
		}
		else if (!res) {
			cout<<"Solution not found. No Solution?"<<std::endl;
		}
		return -1;
	}

	bool validTrain = true;
	if (vm["screen"].as<int>() >= 1) {
	    cout<<"Valid Train Plan: "<< validTrain<<" body conflicts: "<<icbs.num_body_conflict<<" goal conflict: "<<icbs.num_goal_conflict<<" self conflict: "<<icbs.num_self_conflict<<endl;
	}

	// icbs.write_data(vm["output"].as<string>(),vm["agents"].as<string>(),vm["solver"].as<string>(), validTrain);

	if (vm.count("printPath")){
		icbs.printPaths();
	}

	const string & write_path=vm["writePath"].as<string>();
	if (write_path.size()!=0) {
		icbs.writePaths(write_path);
	}

    if(vm.count("statistic")){
        cout<<"Total RM time: "<<icbs.RMTime/CLOCKS_PER_SEC <<endl;
        cout<<"Total time on extract sg: " <<icbs.runtime_findsg/CLOCKS_PER_SEC <<endl;
        cout<<"Total time on find rectangle: " <<icbs.runtime_find_rectangle/CLOCKS_PER_SEC <<endl;
        cout<<"Total time on build mdd in RM: " <<icbs.RMBuildMDDTime/CLOCKS_PER_SEC <<endl;

        cout<<"Total RM detection: "<<icbs.RMDetectionCount <<endl;
        cout<<"Total RM find: "<<icbs.RMSuccessCount <<endl;
        cout<<"Total RM rejected before find rectangle: "<<icbs.RMFailBeforeRec <<endl;
        cout<<"Total MDD: "<<icbs.TotalMDD <<endl;
        cout<<"Total Exist MDD: "<<icbs.TotalExistMDD <<endl;
        cout<<"Total K MDD: "<<icbs.TotalKMDD <<endl;
        cout<<"Total Exist K MDD: "<<icbs.TotalExistKMDD <<endl;
        cout<<"Repeated pairs: "<<icbs.repeated_pairs<<endl;
        for (int i=0; i<icbs.KRectangleCount.size();i++){
            cout<< "total k: "<< i <<" rectangle: " << icbs.KRectangleCount[i]<<endl;
        }
    }

    if (vm.count("pairAnalysis")) {
        icbs.analysisOutput<<"]";
        icbs.analysisOutput.close();
    }

    if (vm["screen"].as<int>() == 2)
		cout << "Done!!" << endl;
	return 0;

}
