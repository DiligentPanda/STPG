#pragma once
#include <vector>
#include <boost/pending/disjoint_sets.hpp>
#include <unordered_map>
#include <unordered_set>
#include "types.h"
#include "ADG/ADG_utilities.h"
#include <iostream>

// TODO(rivers): currently we implement a simple version that only consider independent parallel and crossing patterns.
// and only merge them if they share an edge. A case is clearly missing is concatenation of two such patterns.
// Thus, we are interested in how to further improve the algorithm so that it can finally find all maximal groups. 
// But we doesn't have an algorithm with a solid proof now.
class GroupManager {
public:
    // a set of edge_ids
    using Group=std::unordered_set<int>;
    using Groups=std::vector<Group>;

    Groups groups;
    std::unordered_map<int,int> edge_id2group_id;

    // for simplicity, let's make a copy of adg here
    std::vector<int> states;
    ADG adg;
    Graph & graph;
    int num_states; // used for encoding edge_id=out_state_idx*num_states+in_state_idx

    // pra
    GroupManager(ADG & _adg, std::vector<int> & _states): states(_states), adg(copy_ADG(_adg)), graph(get<0>(adg)), num_states(get<3>(graph)) {
        build();
    };

    int get_edge_id(int out_idx, int in_idx) {
        return out_idx*num_states+in_idx;
    }

    int get_out_idx(int edge_id) {
        return edge_id/num_states;
    }

    int get_in_idx(int edge_id) {
        return edge_id%num_states;
    }

    void build() {
        int num_agents=get_agentCnt(adg);
        // todo: this is not very efficient but fine.
        for (int i=0;i<num_agents;++i) {
            for (int j=0;j<num_agents;++j) {
                if (i!=j) {
                    build_each(i,j);
                }
            }
        }

    }

    void build_each(int out_agent_idx, int in_agent_idx) {
        if (out_agent_idx==in_agent_idx) {
            std::cout<<"two agents cannot be the same"<<std::endl;
            exit(2024);
        }

        auto & accum_cnts=get<2>(adg);

        int out_start_state_idx=compute_vertex(accum_cnts,out_agent_idx,states[out_agent_idx]);
        int out_end_state_idx=accum_cnts[out_agent_idx];

        int in_start_state_idx=compute_vertex(accum_cnts,in_agent_idx,states[in_agent_idx]);
        int in_end_state_idx=accum_cnts[in_agent_idx];

        Groups unmerged_groups;
        std::unordered_set<int> crossing_searched;
        std::unordered_set<int> parallel_searched;
        for (int out_state_idx=out_start_state_idx;out_state_idx<out_end_state_idx;++out_state_idx) {
            auto & switchable_out_neighbors=get_switchable_outNeib(graph, out_state_idx);
            for (int in_state_idx:switchable_out_neighbors) {
                if (in_state_idx>=in_start_state_idx && in_state_idx<in_end_state_idx) {
                    // parallel and crossing patterns
                    find_simple_patterns_starting_with(
                        out_state_idx, in_state_idx,
                        out_end_state_idx, in_start_state_idx, in_end_state_idx,
                        unmerged_groups, crossing_searched, parallel_searched
                    );
                }
            }
        }

        merge_groups(unmerged_groups);
    }

    void find_simple_patterns_starting_with(
        int _out_state_idx, int _in_state_idx, 
        int out_end_state_idx, int in_start_state_idx, int in_end_state_idx,
        Groups & groups, std::unordered_set<int> & crossing_searched, std::unordered_set<int> & parallel_searched
    ) {
        if (crossing_searched.count(get_edge_id(_out_state_idx,_in_state_idx))==0) {
            int out_state_idx=_out_state_idx;
            int in_state_idx=_in_state_idx;
            Group group;
            // search the crossing pattern
            while (true) {
                if (out_state_idx>=out_end_state_idx || in_state_idx<in_start_state_idx) {
                    // reach end
                    break;
                }

                if (!get_type2_switchable_edge(graph,out_state_idx,in_state_idx)) {
                    // no more crossing edge
                    break;
                }

                int edge_id=get_edge_id(out_state_idx,in_state_idx);
                group.emplace(edge_id);
                crossing_searched.emplace(edge_id);
                // check if the next edge exists
                ++out_state_idx;
                --in_state_idx;
            }
            groups.push_back(group);
        }

        if (parallel_searched.count(get_edge_id(_out_state_idx,_in_state_idx))==0)  {
            int out_state_idx=_out_state_idx;
            int in_state_idx=_in_state_idx;
            Group group;
            // serach the parallel pattern
            while (true) {
                if (out_state_idx>=out_end_state_idx || in_state_idx>=in_end_state_idx) {
                    // reach end
                    break;
                }

                if (!get_type2_switchable_edge(graph,out_state_idx,in_state_idx)) {
                    // no more parallel edge
                    break;
                }
                
                int edge_id=get_edge_id(out_state_idx,in_state_idx);
                group.emplace(edge_id);
                crossing_searched.emplace(edge_id);
                // check if the next edge exists
                ++out_state_idx;
                ++in_state_idx;
            }
            groups.push_back(group);
        }
    }

    // We model each group as an vertex. Two groups are adjecent if they share an edge.
    // We use disjoint sets to merge groups. We can ensure that finally each edge is in only one "locally maximal" group.
    void merge_groups(Groups & groups) {

        std::vector<std::pair<int,int> > graph_edges;

        // build graphs
        for (auto i=0;i<groups.size();++i) {
            auto & group_i=groups[i];
            std::unordered_set<int> edge_set(group_i.begin(),group_i.end());
            for (auto j=0;j<groups.size();++j) {
                auto & group_j=groups[j];
                for (int edge_id: group_j) {
                    if (edge_set.count(edge_id)>0) {
                        graph_edges.emplace_back(i,j);
                        break;
                    }
                }
            }
        }

        // use disjoint sets to merge     
        // see https://stackoverflow.com/a/4136546
        // maybe should typedef or using...
        std::unordered_map<int,size_t> rank_map;
        std::unordered_map<int,int> parent_map;

        boost::associative_property_map<std::unordered_map<int,size_t> > rank_pmap(rank_map);
        boost::associative_property_map<std::unordered_map<int,int> > parent_pmap(parent_map);

        std::vector<int> rank(groups.size());
        std::vector<int> parent(groups.size());
        boost::disjoint_sets<
            boost::associative_property_map<std::unordered_map<int,size_t> >,
            boost::associative_property_map<std::unordered_map<int,int> >
        > dsets(rank_pmap,parent_pmap);

        for (int i=0;i<groups.size();++i) {
            dsets.make_set(i);
        }

        for (auto & graph_edge: graph_edges) {
            dsets.union_set(graph_edge.first,graph_edge.second);
        }

        std::unordered_map<int,std::vector<int> > collections;
        for (int i=0;i<groups.size();++i) {
            int collection_id=dsets.find_set(i);
            collections[collection_id].push_back(i);
        }

        for (auto & collection: collections) {
            int big_group_id=this->groups.size();
            this->groups.push_back(Group());
            auto & big_group=this->groups.back();
            auto & group_ids=collection.second;
            for (int group_id:group_ids) {
                auto & group=groups[group_id];
                big_group.insert(group.begin(),group.end());
            }
            for (int edge_id:big_group) {
                this->edge_id2group_id[edge_id]=big_group_id;
            }
        }
    }

    std::unordered_set<int> & get_equivalent_group(int edge_id) {
        if (edge_id2group_id.count(edge_id)==0) {
            std::cout<<"edge_id not found"<<std::endl;
            exit(1207);
        }

        auto & group_id=edge_id2group_id[edge_id];
        return groups[group_id];
    }

    std::vector<std::pair<int,int> > get_groupable_edges(int out_idx, int in_idx) {
        int edge_id=get_edge_id(out_idx, in_idx);
        auto & group=get_equivalent_group(edge_id);

        std::vector<std::pair<int,int> > edges;
        for (auto & edge_id: group) {
            edges.emplace_back(get_out_idx(edge_id),get_in_idx(edge_id));
        }

        return edges;
    }

    void print_group(Group & group) {
        for (auto & edge_id: group) {
            std::cout<<get_out_idx(edge_id)<<"->"<<get_in_idx(edge_id)<<",";
        }
    }

    void print_groups() {
        for (int i=0;i<groups.size();++i) {
            auto & group=groups[i];
            std::cout<<"group "<<i<<": ";
            print_group(group);
            std::cout<<std::endl;
        }
    }

};