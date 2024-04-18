#include "Algorithm/heuristic.h"
#include <unordered_set>

HeuristicManager::HeuristicManager(HeuristicType _type):type(_type) {

}

double HeuristicManager::computeInformedHeuristics(
    const shared_ptr<Graph> & adg, 
    const shared_ptr<vector<int> > & longest_paths, 
    const shared_ptr<vector<shared_ptr<map<int,int> > > > & reverse_longest_paths, 
    double time_limit,
    bool fast_approximate
    ) {
    // TODO: add timer
    int num_of_agents=adg->get_num_agents();

    vector<int> HG(num_of_agents*num_of_agents,0);
    int h=0;

    switch (type) {
        case HeuristicType::ZERO:
            h=0;
            break;
        case HeuristicType::CG_GREEDY:
            buildCardinalConflictGraph(
                adg,
                longest_paths, 
                reverse_longest_paths,
                HG,
                false,
                false
            );
            h=greedyMatching(HG, num_of_agents);
            break;
        case HeuristicType::WCG_GREEDY:
            buildCardinalConflictGraph(
                adg,
                longest_paths, 
                reverse_longest_paths,
                HG,
                true,
                false
            );
            h=greedyMatching(HG, num_of_agents);
            break;
        case HeuristicType::FAST_WCG_GREEDY:
            buildCardinalConflictGraph(
                adg,
                longest_paths, 
                reverse_longest_paths,
                HG,
                true,
                true
            );
            h=greedyMatching(HG, num_of_agents);
            break;
    }
    return h;
}

void HeuristicManager::buildCardinalConflictGraph(
    const shared_ptr<Graph> & adg, 
    const shared_ptr<vector<int> > & longest_paths_ptr, 
    const shared_ptr<vector<shared_ptr<map<int,int> > > > & reverse_longest_paths_ptr,
    vector<int> & CG, 
    bool weighted,
    bool fast_approximate
    ) {
    int num_of_agents=adg->get_num_agents();
    int num_of_states=adg->get_num_states();

    // get all conflicts
    auto last_state_idxs=*(adg->accum_state_cnts_end);
    // get the last state index
    for (int i=0;i<num_of_agents;++i) {
        --last_state_idxs[i];
    }

    auto & longest_paths=*longest_paths_ptr;

    if (fast_approximate) {
        auto & agent_state_cnts=*(adg->state_cnts);
        // TODO(rivers): why would we iterate through all states?
        for (int i=0;i<num_of_states;++i) {
            int i_time=longest_paths[i];
            int agent_i, state_i;
            tie(agent_i, state_i) = adg->get_agent_state_id(i);
            auto & in_state_idxs = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
            for (auto j: in_state_idxs) {
                int j_time=longest_paths[j];
                int back_i=j+1;
                int back_j=i-1;
                int back_i_time=longest_paths[back_i];
                int back_j_time=longest_paths[back_j];
                int diff=i_time-j_time;
                int back_diff=back_i_time-back_j_time;
                if (diff>=0 && back_diff>=0) {
                    int agent_j, state_j;
                    tie(agent_j, state_j) = adg->get_agent_state_id(j);

                    int delta_i=back_i_time+1-back_j_time;
                    int slack_i=longest_paths[last_state_idxs[agent_i]]-back_j_time-(agent_state_cnts[agent_i]-1-state_i);
                    int increase_i=delta_i-slack_i;

                    if (increase_i>0) {
                        int delta_j=i_time+1-j_time;
                        int slack_j=longest_paths[last_state_idxs[agent_j]]-j_time-(agent_state_cnts[agent_j]-1-state_j);
                        int increase_j=delta_j-slack_j;
                        if (increase_j>0) {
                            if (!weighted) {
                                CG[agent_i*num_of_agents+agent_j]=1;
                                CG[agent_j*num_of_agents+agent_i]=1;
                            } else {
                                int cost=std::min(increase_i,increase_j);
                                if (CG[agent_i*num_of_agents+agent_j]<cost) {
                                    CG[agent_i*num_of_agents+agent_j]=cost;
                                    CG[agent_j*num_of_agents+agent_i]=cost;
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        auto & reverse_longest_paths=*reverse_longest_paths_ptr;
        for (int i=0;i<num_of_states;++i) {
            int i_time=longest_paths[i];
            auto & in_state_idxs = adg->switchable_type2_edges->get_out_neighbor_global_ids(i);
            for (auto j: in_state_idxs) {
                int j_time=longest_paths[j];
                int back_i=j+1;
                int back_j=i-1;
                int back_i_time=longest_paths[back_i];
                int back_j_time=longest_paths[back_j];
                int diff=i_time-j_time;
                int back_diff=back_i_time-back_j_time;
                if (diff>=0 && back_diff>=0) {
                    for (auto & pi: *reverse_longest_paths[back_j]) {
                        int a_i=pi.first;
                        int delta_i=back_i_time+1-back_j_time;
                        int slack_i=longest_paths[last_state_idxs[a_i]]-back_j_time-pi.second;
                        int increase_i=delta_i-slack_i;
                        // if (slack_i<0) {
                        //     std::cout<<"Error: slack_i<0"<<std::endl;
                        //     exit(-1);
                        // }
                        if (increase_i>0) {
                            for (auto & pj: *reverse_longest_paths[j]) {
                                int a_j=pj.first;
                                int delta_j=i_time+1-j_time;
                                int slack_j=longest_paths[last_state_idxs[a_j]]-j_time-pj.second;
                                int increase_j=delta_j-slack_j;
                                if (increase_j>0) {
                                    if (!weighted) {
                                        CG[a_i*num_of_agents+a_j]=1;
                                        CG[a_j*num_of_agents+a_i]=1;
                                    } else {
                                        int cost=std::min(increase_i,increase_j);
                                        if (CG[a_i*num_of_agents+a_j]<cost) {
                                            CG[a_i*num_of_agents+a_j]=cost;
                                            CG[a_j*num_of_agents+a_i]=cost;
                                        }
                                    }
                                }
                                // if (slack_j<0) {
                                //     std::cout<<"Error: slack_j<0"<<std::endl;
                                //     exit(-1);
                                // }
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


