#include "Algorithm/heuristic.h"
#include "ADG/ADG_utilities.h"
#include "graph/graph.h"
#include <unordered_set>

HeuristicManager::HeuristicManager(HeuristicType _type):type(_type) {

}

double HeuristicManager::computeInformedHeuristics(ADG & adg, vector<int> & tp_ordered_states, vector<int> & tp_times, double time_limit) {
    // TODO: add timer
    int num_of_agents=get_agentCnt(adg);

    vector<int> HG(num_of_agents*num_of_agents,0);
    int h=0;

    switch (type) {
        case HeuristicType::ZERO:
            h=0;
            break;
        case HeuristicType::CG_GREEDY:
            buildCardinalConflictGraph(adg,tp_ordered_states,tp_times,HG,false);
            h=greedyMatching(HG, num_of_agents);
            break;
        case HeuristicType::WCG_GREEDY:
            buildCardinalConflictGraph(adg,tp_ordered_states,tp_times,HG,true);
            h=greedyMatching(HG, num_of_agents);
            break;
    }
    return h;
}

void HeuristicManager::buildCardinalConflictGraph(ADG & adg, vector<int> & tp_ordered_states, vector<int> & tp_times, vector<int> & CG, bool weighted) {
    int num_of_agents=get_agentCnt(adg);
    auto & graph=get<0>(adg);
    int num_of_states=get<3>(graph);

    // TODO(rivers): this can computed during the forward computation.
    // a map from a state to a set of agent indexs.
    std::vector<std::unordered_set<int> > on_whose_critical_paths(num_of_states);

    auto & accum_state_cnts=(*get<2>(adg));

    // initialize the last state for each agent
    for (auto i=0;i<num_of_agents;++i) {
       // the last vertex is always on its own agent's critical path
       int last_state_idx=accum_state_cnts[i]-1;
       on_whose_critical_paths[last_state_idx].insert(i); 
    }

    // we will fill in the results in the reverse order
    for (int i=(int)tp_ordered_states.size()-1;i>=0;--i) {
        int state_idx=tp_ordered_states[i];
        if (on_whose_critical_paths[state_idx].size()==0) {
            auto successors=get_nonSwitchable_outNeib(graph, state_idx);
            for (auto successor: successors) {
                // NOTE(rivers): this is specific to the discrete-time case, where time diff==1 means no wait.
                if (tp_times[state_idx]+1==tp_times[successor]) {
                    on_whose_critical_paths[state_idx].insert(
                        on_whose_critical_paths[successor].begin(),
                        on_whose_critical_paths[successor].end()
                    );
                }
            }
        }
    }

    // get all conflicts
    for (int i=0;i<get<3>(graph);++i) {
        int i_time=tp_times[i];
        set<int> & in_state_idxs = get_switchable_outNeib(graph, i);
        for (auto j: in_state_idxs) {
            int j_time=tp_times[j];
            int back_i=j+1;
            int back_j=i-1;
            int back_i_time=tp_times[back_i];
            int back_j_time=tp_times[back_j];
            int diff=i_time-j_time;
            int back_diff=back_i_time-back_j_time;
            if (diff>=0 && back_diff>=0) {
                for (auto ai: on_whose_critical_paths[back_j]) {
                    for (auto aj: on_whose_critical_paths[j]) {
                        if (!weighted) {
                            CG[ai*num_of_agents+aj]=1;
                            CG[aj*num_of_agents+ai]=1;
                        } else {
                            int cost=std::min(diff,back_diff)+1;
                            if (CG[ai*num_of_agents+aj]<cost) {
                                CG[ai*num_of_agents+aj]=cost;
                                CG[aj*num_of_agents+ai]=cost;
                            }
                        }
                    }
                }
            }
        }
    }

}

double HeuristicManager::greedyMatching(const std::vector<int> & CG, int num_vertices) {
	double rst = 0;
	std::vector<bool> used(num_vertices, false);
	while(true)
	{
		int maxWeight = 0;
		int ep1, ep2;
		for (int i = 0; i < num_vertices; i++)
		{
			if(used[i])
				continue;
			for (int j = i + 1; j < num_vertices; j++)
			{
				if (used[j])
					continue;
				else if (maxWeight < CG[i * num_vertices + j])
				{
					maxWeight = CG[i * num_vertices + j];
					ep1 = i;
					ep2 = j;
				}
			}
		}
		if (maxWeight == 0)
			return rst;
		rst += maxWeight;
		used[ep1] = true;
		used[ep2] = true;
	}
    // impossible to reach here
    return rst;
}


