#pragma once
#include <vector>
#include <boost/pending/disjoint_sets.hpp>
#include <unordered_map>
#include <unordered_set>
#include "graph/graph.h"
#include <iostream>
#include "util/util.h"

enum GroupingMethod {
    NONE,
    SIMPLE,
    SIMPLE_MERGE,
    ALL
};

// TODO(rivers): currently we implement a simple version that only consider independent parallel and crossing patterns.
// and only merge them if they share an edge. A case is clearly missing is concatenation of two such patterns.
// Thus, we are interested in how to further improve the algorithm so that it can finally find all maximal groups. 
// But we doesn't have an algorithm with a solid proof now.
class GroupManager {
public:
    // a set of edge_ids
    // TODO(rivers): we should make the data type pair of int directly by pair int hash...
    using Group=unordered_set<long>;
    using Groups=vector<Group>;

    Groups groups;
    unordered_map<long,int> edge_id2group_id;

    // for simplicity, let's make a copy of graph here
    vector<int> states;
    shared_ptr<Graph> graph;
    int num_states; // used for encoding edge_id=out_state_idx*num_states+in_state_idx

    // we don't count any merge with size 1 group, which is just a single edge.
    int group_merge_edge_cnt=0;

    GroupingMethod grouping_method;

    GroupManager(const shared_ptr<Graph> & _graph, vector<int> & _states, const string & _grouping_method): states(_states), graph(_graph), num_states(_graph->get_num_states()) {

        if (_grouping_method=="none") {
            grouping_method=GroupingMethod::NONE;
        } else if (_grouping_method=="simple") {
            grouping_method=GroupingMethod::SIMPLE;
        } else if (_grouping_method=="simple_merge") {
            grouping_method=GroupingMethod::SIMPLE_MERGE;
        } else if (_grouping_method=="all") {
            grouping_method=GroupingMethod::ALL;
        } else {
            std::cout<<"unknown grouping method: "<<_grouping_method<<std::endl;
            exit(19);
        }

        if (grouping_method==GroupingMethod::SIMPLE) {
            build();
        } else if (grouping_method==GroupingMethod::SIMPLE_MERGE) {
            build();
        } else if (grouping_method==GroupingMethod::ALL) {
            build2();
        } else {
            cout<<"unknown method for dependency grouping"<<endl;
            exit(1234);
        }

    };

    // BUG(rivers): don't use int to encode, would overflow in the future.
    long get_edge_id(int out_idx, int in_idx) {
        return (long)out_idx*(long)num_states+(long)in_idx;
    }

    int get_out_idx(long edge_id) {
        return (int)(edge_id/num_states);
    }

    int get_in_idx(long edge_id) {
        return (int)(edge_id%num_states);
    }

    void build2() {
        int num_agents=graph->get_num_agents();

        // todo: this is not very efficient but fine.
        for (int i=0;i<num_agents;++i) {
            for (int j=0;j<num_agents;++j) {
                if (i!=j) {
                    build_each2(i,j);
                }
            }
        }
        // print_groups();
    }

    inline pair<int,int> reverse_edge(const pair<int,int> & edge) {
        return {edge.second+1,edge.first-1};
    }

    void build_each2(int out_agent_idx, int in_agent_idx) {
        if (out_agent_idx==in_agent_idx) {
            cout<<"two agents cannot be the same"<<endl;
            exit(2024);
        }

        auto & prev_accum_cnts=*(graph->accum_state_cnts_begin);
        auto & state_cnts=*(graph->state_cnts);

        int out_start_state_local_idx=states[out_agent_idx];
        // excluded
        int out_end_state_local_idx=state_cnts[out_agent_idx];

        int in_start_state_local_idx=states[in_agent_idx];
        // excluded
        int in_end_state_local_idx=state_cnts[in_agent_idx];

        unordered_set<pair<int,int>, PairIntHash> all_edges;
        // TODO: we should consider non-switchable edges in the future.
        for (int out_state_local_idx=out_start_state_local_idx;out_state_local_idx<out_end_state_local_idx;++out_state_local_idx) {
            int out_state_idx=out_state_local_idx+prev_accum_cnts[out_agent_idx];
            auto & switchable_out_neighbors=graph->switchable_type2_edges->get_out_neighbor_global_ids(out_state_idx);
            for (int in_state_idx:switchable_out_neighbors) {
                int in_state_local_idx=in_state_idx-prev_accum_cnts[in_agent_idx];
                if (in_state_local_idx>=in_start_state_local_idx && in_state_local_idx<in_end_state_local_idx) {
                    all_edges.emplace(out_state_local_idx,in_state_local_idx);
                }
            }
        }

        unordered_set<pair<int,int>, PairIntHash > all_reversed_edges;
        for (auto & edge: all_edges) {
            auto reversed_edge=reverse_edge(edge);
            all_reversed_edges.insert(reversed_edge);
        }
        

        while (all_edges.size()>0) {
            auto edge=*all_edges.begin();
            // cout<<"edge: "<<edge.first<<"->"<<edge.second<<endl;

            unordered_set<pair<int,int>, PairIntHash> forward_all_edges(all_edges);
            unordered_set<pair<int,int>, PairIntHash > forward_groupable_edges;

            unordered_set<pair<int,int>, PairIntHash> backward_all_edges(all_reversed_edges);
            unordered_set<pair<int,int>, PairIntHash> backward_groupable_edges;
            
            {
                forward_all_edges.erase(edge);
                forward_groupable_edges.insert(edge);

                // TODO(rivers): this implementation is not efficient?
                vector<pair<int,int>> reversed_edges={reverse_edge(edge)};
                while (true) {
                    unordered_set<pair<int,int>, PairIntHash> edges_to_reverse;
                    for (auto & forward_edge: forward_all_edges) {
                        for (auto & reversed_edge:reversed_edges) {
                            if (forward_edge.first>=reversed_edge.second && forward_edge.second<=reversed_edge.first){
                                edges_to_reverse.insert(forward_edge);
                                break;
                            }
                        }
                    }
                    if (edges_to_reverse.size()==0) {
                        break;
                    }
                    reversed_edges.clear();
                    for (auto & edge_to_reverse:edges_to_reverse) {
                        forward_all_edges.erase(edge_to_reverse);
                        forward_groupable_edges.insert(edge_to_reverse);
                        reversed_edges.push_back(reverse_edge(edge_to_reverse));
                    }
                }

                // cout<<"forward_group_edges"<<endl;
                // for (auto edge:forward_groupable_edges) {
                //     cout<<edge.first<<"->"<<edge.second<<endl;
                // }
            }

            {
                edge=reverse_edge(edge);
                
                backward_all_edges.erase(edge);
                backward_groupable_edges.insert(edge);

                vector<pair<int,int>> reversed_edges={reverse_edge(edge)};
                while (true) {
                    unordered_set<pair<int,int>, PairIntHash> edges_to_reverse;
                    for (auto & backward_edge: backward_all_edges) {
                        for (auto & reversed_edge:reversed_edges) {
                            if (backward_edge.first>=reversed_edge.second && backward_edge.second<=reversed_edge.first){
                                edges_to_reverse.insert(backward_edge);
                                break;
                            }
                        }
                    }
                    if (edges_to_reverse.size()==0) {
                        break;
                    }
                    reversed_edges.clear();
                    for (auto & edge_to_reverse:edges_to_reverse) {
                        backward_all_edges.erase(edge_to_reverse);
                        backward_groupable_edges.insert(edge_to_reverse);
                        reversed_edges.push_back(reverse_edge(edge_to_reverse));
                    }
                }

                // cout<<"backward_group_edges"<<endl;
                // for (auto edge:backward_groupable_edges) {
                //     cout<<edge.first<<"->"<<edge.second<<endl;
                // }
            }

            
            // C++11 (needs <unordered_set>)
            unordered_set<pair<int,int>, PairIntHash> groupable_edges;
            // TODO: consider reserving a size (optimistically |a| + |b|)
            for (auto & edge : forward_groupable_edges) {
                auto reversed_edge=reverse_edge(edge);
                if (backward_groupable_edges.count(reversed_edge)) { 
                    groupable_edges.insert(edge);
                    all_edges.erase(edge);
                    all_reversed_edges.erase(reversed_edge); 
                }
            }

            Group group;
            for (auto edge:groupable_edges) {
                long edge_id=get_edge_id(edge.first+prev_accum_cnts[out_agent_idx],edge.second+prev_accum_cnts[in_agent_idx]);
                group.insert(edge_id);
            }
            int group_id=groups.size();
            groups.push_back(group);
            for (long edge_id:group) {
                edge_id2group_id[edge_id]=group_id;
            }
        }

    }

    void build() {
        int num_agents=graph->get_num_agents();
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
            cout<<"two agents cannot be the same"<<endl;
            exit(2024);
        }

        auto & accum_cnts=*(graph->accum_state_cnts_end);

        int out_start_state_idx=graph->get_global_state_id(out_agent_idx,states[out_agent_idx]);
        int out_end_state_idx=accum_cnts[out_agent_idx];

        int in_start_state_idx=graph->get_global_state_id(in_agent_idx,states[in_agent_idx]);
        int in_end_state_idx=accum_cnts[in_agent_idx];

        Groups unmerged_groups;
        unordered_set<long> crossing_searched;
        unordered_set<long> parallel_searched;
        for (int out_state_idx=out_start_state_idx;out_state_idx<out_end_state_idx;++out_state_idx) {
            auto & switchable_out_neighbors=graph->switchable_type2_edges->get_out_neighbor_global_ids(out_state_idx);
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
        Groups & groups, unordered_set<long> & crossing_searched, unordered_set<long> & parallel_searched
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

                if (!graph->switchable_type2_edges->has_edge(out_state_idx,in_state_idx)) {
                    // no more crossing edge
                    break;
                }

                long edge_id=get_edge_id(out_state_idx,in_state_idx);
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

                if (!graph->switchable_type2_edges->has_edge(out_state_idx,in_state_idx)) {
                    // no more parallel edge
                    break;
                }
                
                long edge_id=get_edge_id(out_state_idx,in_state_idx);
                group.emplace(edge_id);
                parallel_searched.emplace(edge_id);
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
        vector<pair<int,int> > graph_edges;

        // build graphs
        if (grouping_method==GroupingMethod::SIMPLE_MERGE) {
            for (size_t i=0;i<groups.size();++i) {
                auto & group_i=groups[i];
                unordered_set<int> edge_set(group_i.begin(),group_i.end());
                for (size_t j=i+1;j<groups.size();++j) {
                    auto & group_j=groups[j];
                    for (long edge_id: group_j) {
                        if (edge_set.count(edge_id)>0) {
                            graph_edges.emplace_back(i,j);
                            if (group_i.size()>1 && group_j.size()>1) {
                                // cout<<"group i: "<<group_i.size()<<" group j: "<<group_j.size()<<" "<<group_merge_edge_cnt<<endl;
                                // cout<<"group i: "<<endl;
                                // for (auto edge_id: group_i) {
                                //     cout<<get_out_idx(edge_id)<<"->"<<get_in_idx(edge_id)<<",";
                                // }
                                // cout<<"group j: "<<endl;
                                // for (auto edge_id: group_j) {
                                //     cout<<get_out_idx(edge_id)<<"->"<<get_in_idx(edge_id)<<",";
                                // }
                                ++group_merge_edge_cnt;
                            }
                            break;
                        }
                    }
                }
            }
        }

        // use disjoint sets to merge     
        // see https://stackoverflow.com/a/4136546
        // maybe should typedef or using...
        unordered_map<int,size_t> rank_map;
        unordered_map<int,int> parent_map;

        boost::associative_property_map<unordered_map<int,size_t> > rank_pmap(rank_map);
        boost::associative_property_map<unordered_map<int,int> > parent_pmap(parent_map);

        vector<int> rank(groups.size());
        vector<int> parent(groups.size());
        boost::disjoint_sets<
            boost::associative_property_map<unordered_map<int,size_t> >,
            boost::associative_property_map<unordered_map<int,int> >
        > dsets(rank_pmap,parent_pmap);

        for (int i=0;i<(int)groups.size();++i) {
            dsets.make_set(i);
        }

        for (auto & graph_edge: graph_edges) {
            dsets.union_set(graph_edge.first,graph_edge.second);
        }

        unordered_map<int,vector<int> > collections;
        for (int i=0;i<(int)groups.size();++i) {
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
            for (long edge_id:big_group) {
                this->edge_id2group_id[edge_id]=big_group_id;
            }
        }
    }

    int get_group_id(int out_idx, int in_idx) {
        long edge_id=get_edge_id(out_idx, in_idx);
        auto itr=edge_id2group_id.find(edge_id);
        if (itr==edge_id2group_id.end()) {
            return -1;
        }
        return itr->second;
    }

    bool get_equivalent_group(int out_idx, int in_idx, unordered_set<long> & group) {
        int group_id=get_group_id(out_idx, in_idx);
        if (group_id==-1) {
            return false;
        }
        group=groups[group_id];
        return true;
    }

    vector<pair<int,int> > get_groupable_edges(int out_idx, int in_idx) {
        unordered_set<long> group;
        bool found=get_equivalent_group(out_idx,in_idx,group);
        if (!found) {
            int agent_idx,state_idx;
            tie(agent_idx,state_idx)=graph->get_agent_state_id(out_idx);
            cout<<"out_idx: agent_idx="<<agent_idx<<",state_idx="<<state_idx<<",current states:"<<states[agent_idx]
            <<". ("<<(*graph->paths)[agent_idx][state_idx].first.first<<","<<(*graph->paths)[agent_idx][state_idx].first.second<<")"<<","<<(*graph->paths)[agent_idx][state_idx].second<<endl;
            tie(agent_idx,state_idx)=graph->get_agent_state_id(in_idx);
            cout<<"in_idx: agent_idx="<<agent_idx<<",state_idx="<<state_idx<<",current states:"<<states[agent_idx]
            <<". ("<<(*graph->paths)[agent_idx][state_idx].first.first<<","<<(*graph->paths)[agent_idx][state_idx].first.second<<")"<<","<<(*graph->paths)[agent_idx][state_idx].second<<endl;
            exit(1234);
        }

        vector<pair<int,int> > edges;
        for (auto & edge_id: group) {
            edges.emplace_back(get_out_idx(edge_id),get_in_idx(edge_id));
        }

        return edges;
    }

    vector<pair<int,int> > get_groupable_edges(int group_id) {
        if (group_id<0 || group_id>=(int)groups.size()) {
            cout<<"group_id out of range"<<endl;
            exit(1234);
        }

        auto & group=groups[group_id];

        vector<pair<int,int> > edges;
        for (auto & edge_id: group) {
            edges.emplace_back(get_out_idx(edge_id),get_in_idx(edge_id));
        }

        return edges;
    }

    void print_group(Group & group) {
        for (auto & edge_id: group) {
            cout<<get_out_idx(edge_id)<<"->"<<get_in_idx(edge_id)<<",";
        }
    }

    void print_groups() {
        for (size_t i=0;i<groups.size();++i) {
            auto & group=groups[i];
            cout<<"group "<<i<<": ";
            print_group(group);
            cout<<endl;
        }
    }

};