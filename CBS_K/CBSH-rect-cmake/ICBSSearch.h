#pragma once
//#ifndef CBS_K_ICBSSEARCH_H
//#define CBS_K_ICBSSEARCH_H
//#endif

#include "ICBSNode.h"
#include "SingleAgentICBS.h"
#include "compute_heuristic.h"
#include "constrained_agents_loader.h"
#include "RectangleReasoning.h"
#include "CorridorReasoning.h"
#include "ConstraintTable.h"
#include "common.h"
#include "MDD.h"
#include "Path.h"
#include <unordered_map>
//#include <boost/python.hpp>
#include <fstream>


class ICBSSearch
{
public:
	double runtime = 0;
	double runtime_lowlevel;
	double runtime_conflictdetection;
	double runtime_computeh;
	double runtime_listoperation;
	double runtime_updatepaths;
	double runtime_updatecons;
	double RMTime = 0;
	double runtime_findsg;
	double runtime_find_rectangle;
	double RMBuildMDDTime;
	double runtime_mdd = 0;

	int RMDetectionCount=0;
	int RMSuccessCount=0;
	int RMFailBeforeRec=0;
	int TotalMDD=0;
	int TotalKMDD=0;
	int TotalExistMDD=0;
	int TotalExistKMDD=0;
	vector<int> KRectangleCount;

	

	ICBSNode* dummy_start;

	uint64_t HL_num_expanded = 0;
	uint64_t HL_num_generated = 0;
	uint64_t LL_num_expanded = 0;
	uint64_t LL_num_generated = 0;

	uint64_t num_corridor2 = 0;
	uint64_t num_corridor4 = 0;
	uint64_t num_rectangle = 0;
	uint64_t num_target_std = 0;
	uint64_t num_target_sym = 0;
	uint64_t num_parking_std = 0;
	uint64_t num_parking_sym = 0;
	uint64_t num_standard = 0;
	uint64_t num_standard_m2 = 0;

    uint64_t num_train_standard = 0;
    uint64_t num_train_self = 0;

    uint64_t num_cardinal = 0;
    uint64_t num_semicardinal = 0;
    uint64_t num_noncardinal = 0;
    uint64_t num_unkown = 0;


    uint64_t num_lltp = 0;
    uint64_t num_llpp = 0;


	bool solution_found;
	int solution_cost;
	double min_f_val;
	double focal_list_threshold;
	bool cardinalRect = false;
	bool rectangleMDD = false;
	bool corridor2 = false;
	bool corridor4 = false;
	bool cardinalCorridorReasoning = false;
	bool targetReasoning=false;
	int kDelay;
    int screen;
	int numOfRectangle = 0;
	bool debug_mode=false;
	bool I_RM = false;
	std::clock_t start;
    int num_col;


    options option;

    int num_body_conflict = 0;
    int num_goal_conflict = 0;
    int num_self_conflict = 0;


    void printBT(const std::string& prefix, const ICBSNode* node, bool isLeft);
	void printHLTree();

    vector<vector<PathEntry>*> getPaths(){ return paths;};
	// print
	void printPaths() const;
	void printPaths(Path& path) const;
	void writePaths(const string & s) const;



    ICBSSearch() {};
    ConstrainedAgentsLoader al;


protected:
	
	typedef boost::heap::fibonacci_heap< ICBSNode*, boost::heap::compare<ICBSNode::compare_node> > heap_open_t;
	typedef boost::heap::fibonacci_heap< ICBSNode*, boost::heap::compare<ICBSNode::secondary_compare_node> > heap_focal_t;
	heap_open_t open_list;
	heap_focal_t focal_list;
	list<ICBSNode*> allNodes_table;

	CorridorTable corridorTable;

	ConstraintTable constraintTable;

	constraint_strategy cons_strategy;
	int time_limit;
	int node_limit = 0;
	double focal_w = 1.0;

	const bool* my_map;
	int map_size;
	int num_of_agents;
	const int* actions_offset;
	const int* moves_offset;

	vector<vector<PathEntry>*> paths;
	vector<vector<PathEntry>> paths_found_initially;  // contain initial paths found
	
	virtual bool findPathForSingleAgent(ICBSNode*  node, int ag, double lowerbound = 0) {return true;};
	virtual void  classifyConflicts(ICBSNode &parent) {};
	void findTargetConflicts(int a1, int a2, ICBSNode& curr);

	// high level search
	bool generateChild(ICBSNode* child, ICBSNode* curr);
	//conflicts
	void findConflicts(ICBSNode& curr);
	void findConflicts2All(ICBSNode& curr, int a1);
	std::shared_ptr<Conflict> chooseConflict(ICBSNode &parent);
	void copyConflicts(const std::list<std::shared_ptr<Conflict>>& conflicts,
		std::list<std::shared_ptr<Conflict>>& copy, const list<int>& excluded_agent) const;
	// void copyConflicts(const std::list<std::shared_ptr<CConflict>>& conflicts,
	// 	std::list<std::shared_ptr<CConflict>>& copy, int excluded_agent) const;
	// void deleteRectConflict(ICBSNode& curr, const Conflict& conflict);
	bool hasCardinalConflict(const ICBSNode& node) const;
	bool blocked(const Path& path, const std::list<Constraint>& constraint) const;
	bool traverse(const Path& path, int loc, int t) const;
	void removeLowPriorityConflicts(std::list<std::shared_ptr<Conflict>>& conflicts) const;

	// add heuristics for the high-level search
	int computeHeuristics(const ICBSNode& curr);
	bool KVertexCover(const vector<vector<bool>>& CG, int num_of_CGnodes, int num_of_CGedges, int k);
	int minimumVertexCover(const vector<vector<bool>>& CG);
	int greedyMatching(const vector<vector<bool>>& CG);

	//update information
	// vector < list< pair<int, int> > >* collectConstraints(ICBSNode* curr, int agent_id);
	virtual void updateConstraintTable(ICBSNode* curr, int agent_id) {};
	inline void updatePaths(ICBSNode* curr);
	void updateFocalList(double old_lower_bound, double new_lower_bound, double f_weight);
	void updateReservationTable(bool* res_table, int exclude_agent, const ICBSNode &node);
	inline void releaseClosedListNodes(){
        for (list<ICBSNode*>::iterator it = allNodes_table.begin(); it != allNodes_table.end(); it++)
            delete *it;
        allNodes_table.clear();
    };
	inline void releaseOpenListNodes(){
        while(!open_list.empty())
        {
            ICBSNode* curr = open_list.top();
            open_list.pop();
            delete curr;
        }
    };




    void printStrategy() const;
	bool timeout=false;


    void finalConflictCheck(ICBSNode& curr);
    void conflict_between_2(ICBSNode& curr, int a1, int a2);





};

template<class Map>
class MultiMapICBSSearch :public ICBSSearch
{
public:
    MultiMapICBSSearch(){};
	MultiMapICBSSearch(Map * ml, ConstrainedAgentsLoader & al, double f_w, constraint_strategy c, int time_limit, int screen,int kDlay, options options1);	
	// build MDD
	MDD<Map>* buildMDD(ICBSNode& node, int id, int k=0, bool train = false);
	void updateConstraintTable(ICBSNode* cTurr, int agent_id);
	void classifyConflicts(ICBSNode &parent);
	bool have_delta_conflict(ICBSNode &node);
	void initializeDummyStart();
	bool isCorridorConflict(std::shared_ptr<Conflict>& corridor, const std::shared_ptr<Conflict>& con, ICBSNode* node);
	bool rectangleReasoning(const std::shared_ptr<Conflict>& con,ICBSNode &parent,std::shared_ptr<Conflict>& rectangle);

	bool findPathForSingleAgent(ICBSNode*  node, int ag, double lowerbound = 0);
	// Runs the algorithm until the problem is solved or time is exhausted 
	bool runICBSSearch();
    bool search();
    void checkRepeatance(ICBSNode* curr);

    ~MultiMapICBSSearch();
//	boost::python::list outputPaths()
//	{
//		boost::python::list result;
//		for (int i = 0; i < num_of_agents; i++)
//		{
//			boost::python::list agentPath;
//
//
//			for (int t = 0; t < paths[i]->size(); t++) {
//				boost::python::tuple location = boost::python::make_tuple(paths[i]->at(t).location / num_col, paths[i]->at(t).location % num_col, paths[i]->at(t).actionToHere);
//				agentPath.append(location);
//			}
//			result.append(agentPath);
//		}
//		return result;
//	}
	bool isTimeout() { return timeout; };

	bool trainCorridor1 = false;
	bool trainCorridor2 = false;
    vector<SingleAgentICBS<Map> *> search_engines;  // used to find (single) agents' paths and mdd

    MultiMapICBSSearch<Map>* analysisEngine = NULL;
    void startPairAnalysis(ICBSNode* node,int agent1, int agent2);
    virtual void clear(){};
    virtual bool pairedAnalysis(ICBSNode* node,int agent1, int agent2){return true;};
    void countNodes(int amount);
    void printConstraints(ICBSNode* node,int agent_id,ofstream& out);
    void print_data();
    void write_data(string ouput_file, string agents_file, string solver, bool validTrain);

    ofstream analysisOutput;
    string analysisOutputPath;

    int less10 = 0;
    int less100 = 0;
    int less1000 = 0;
    int less10000 = 0;
    int less100000 = 0;
    int larger100000 = 0;
    int num_pairs = 0;
    int num_failed = 0;
    int repeated_pairs = 0;
    bool analysisInstance = false;



protected:
	std::vector<std::unordered_map<ConstraintsHasher, MDD<Map>*>> mddTable;
    std::vector<std::unordered_map<ConstraintsHasher, std::unordered_set<ConstraintsHasher>>> pairAnalysisTable;

	Map* ml;
};



