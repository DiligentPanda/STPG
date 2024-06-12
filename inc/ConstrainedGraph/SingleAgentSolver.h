#pragma once
#include "Instance.h"
#include "DelayInstance.h"
#include "ConstrainedGraph/ConstraintTable.h"

namespace ConstrainedGraph {

class LLNode // low-level node
{
public:
	Location Loc{};
	int g_val = 0;
	int h_val = 0;
	LLNode* parent = nullptr;
	int timestep = 0;
	int num_of_conflicts = 0;
	bool in_openlist = false;
	bool wait_at_goal = false; // the action is to wait at the goal vertex or not. This is used for >lenghth constraints
    bool is_goal = false;
	// the following is used to comapre nodes in the OPEN list
	struct compare_node
	{
		// returns true if n1 > n2 (note -- this gives us *min*-heap).
		bool operator()(const LLNode* n1, const LLNode* n2) const
		{
            if (n1->g_val + n1->h_val == n2->g_val + n2->h_val)
            {
                if (n1->h_val == n2->h_val)
                {
                    return rand() % 2 == 0;   // break ties randomly
                }
                return n1->h_val >= n2->h_val;  // break ties towards smaller h_vals (closer to goal location)
            }
			return n1->g_val + n1->h_val >= n2->g_val + n2->h_val;
		}
	};  // used by OPEN (heap) to compare nodes (top of the heap has min f-val, and then highest g-val)

		// the following is used to compare nodes in the FOCAL list
	struct secondary_compare_node
	{
		bool operator()(const LLNode* n1, const LLNode* n2) const // returns true if n1 > n2
		{
			if (n1->num_of_conflicts == n2->num_of_conflicts)
			{
                if (n1->g_val + n1->h_val == n2->g_val + n2->h_val)
                {
                    if (n1->h_val == n2->h_val)
                    {
                        return rand() % 2 == 0;   // break ties randomly
                    }
                    return n1->h_val >= n2->h_val;  // break ties towards smaller h_vals (closer to goal location)
                }
                return n1->g_val+n1->h_val >= n2->g_val+n2->h_val;  // break ties towards smaller f_vals (prefer shorter solutions)
			}
			return n1->num_of_conflicts >= n2->num_of_conflicts;  // n1 > n2 if it has more conflicts
		}
	};  // used by FOCAL (heap) to compare nodes (top of the heap has min number-of-conflicts)


	LLNode() {}
	LLNode(Location location, int g_val, int h_val, LLNode* parent, int timestep, int num_of_conflicts) :
		Loc(location), g_val(g_val), h_val(h_val), parent(parent), timestep(timestep),
		num_of_conflicts(num_of_conflicts) {}
	LLNode(const LLNode& other) { copy(other); }

	void copy(const LLNode& other)
	{
		Loc = other.Loc;
		g_val = other.g_val;
		h_val = other.h_val;
		parent = other.parent;
		timestep = other.timestep;
		num_of_conflicts = other.num_of_conflicts;
		wait_at_goal = other.wait_at_goal;
		is_goal = other.is_goal;
	}
    inline int getFVal() const { return g_val + h_val; }
};

std::ostream& operator<<(std::ostream& os, const LLNode& node);

class SingleAgentSolver
{
public:
    uint64_t accumulated_num_expanded = 0;
    uint64_t accumulated_num_generated = 0;
    uint64_t accumulated_num_reopened = 0;
    uint64_t num_runs = 0;

    int num_collisions = -1;
	double runtime_build_CT = 0; // runtimr of building constraint table
	double runtime_build_CAT = 0; // runtime of building conflict avoidance table

	Location start_location;
	Location goal_location;
	vector<int> my_heuristic;  // this is the precomputed heuristic for this agent
	int compute_heuristic(int from, int to) const  // compute admissible heuristic between two locations
	{
		return max(get_DH_heuristic(from, to), (*instance).getManhattanDistance(from, to));
	}
	Instance *instance;

    //virtual Path findOptimalPath(const PathTable& path_table) = 0;
    //virtual Path findOptimalPath(const ConstraintTable& constraint_table, const PathTableWC& path_table) = 0;
	virtual Path findOptimalPath(const HLNode& node, const ConstraintTable& initial_constraints,
		const vector<Path*>& paths, int agent, int lower_bound) = 0;
	virtual pair<Path, int> findSuboptimalPath(const HLNode& node, const ConstraintTable& initial_constraints,
		const vector<Path*>& paths, int agent, int lowerbound, double w) = 0;  // return the path and the lowerbound
    virtual Path findPath(const ConstraintTable& constraint_table) = 0;  // return the path
    void findMinimumSetofColldingTargets(vector<int>& goal_table,set<int>& A_target);
    virtual int getTravelTime(int start, int end, const ConstraintTable& constraint_table, int upper_bound) = 0;
	virtual string getName() const = 0;

	list<Location> getNextLocations(Location curr) const; // including itself and its neighbors
	list<Location> getNeighbors(Location curr) const 
	{ 
		return instance->getNeighbors(curr); 
	}
    uint64_t getNumExpanded() const { return num_expanded; }
	// int getStartLocation() const {return instance.start_locations[agent]; }
	// int getGoalLocation() const {return instance.goal_locations[agent]; }

	SingleAgentSolver(Instance *instance, int agent) :
		instance(instance), agent(agent), 
		start_location((*instance).start_locations[agent]),
		goal_location((*instance).goal_locations[agent])
	{
		compute_heuristics();
	}

	SingleAgentSolver(Instance *instance, int agent, Path &path) :
		instance(instance), agent(agent), 
		start_location((*instance).start_locations[agent].location, (*instance).start_locations[agent].index),
		goal_location((*instance).goal_locations[agent])
	{
		// std::cout << "in init: " << start_location.location << " " << start_location.index << std::endl;
		instance->agent_idx = agent; // only used by DelayInstance
		compute_postDelay_heuristics(path);
	}

	const int agent;
	virtual ~SingleAgentSolver(){}
    void reset()
    {
        if (num_generated > 0)
        {
            accumulated_num_expanded += num_expanded;
            accumulated_num_generated += num_generated;
            accumulated_num_reopened += num_reopened;
            num_runs++;
        }
        num_expanded = 0;
        num_generated = 0;
        num_reopened = 0;
        instance->agent_idx = agent;
    }
protected:
    uint64_t num_expanded = 0;
    uint64_t num_generated = 0;
    uint64_t num_reopened = 0;
	int min_f_val; // minimal f value in OPEN
	// int lower_bound; // Threshold for FOCAL
	double w = 1; // suboptimal bound

	void compute_heuristics();
	void compute_postDelay_heuristics(Path &path);
	int get_DH_heuristic(int from, int to) const { return abs(my_heuristic[from] - my_heuristic[to]); }
};

};