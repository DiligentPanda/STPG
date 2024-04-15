import pandas as pd
import os
import numpy as np
import json
import argparse

arg_parser=argparse.ArgumentParser()
arg_parser.add_argument("-f","--folder",type=str,default="")
arg_parser.add_argument("-c","--check_missing",action="store_true")
arg_parser.add_argument("-s","--only_all_solved",action="store_true")
arg_parser.add_argument("-g","--group_agent_num",action="store_true")

args=arg_parser.parse_args()

check_missing=args.check_missing
only_all_solved=args.only_all_solved
group_agent_num=args.group_agent_num

result_folder=args.folder
if not os.path.isdir(result_folder):
    print(result_folder, "doesn't exist!")

stat_folder=os.path.join(result_folder,"stat")
stats_ofp=os.path.join(
    result_folder,"stats{}.csv".format(
        "_solved" if only_all_solved else "_all"
    )
)
stat_summary_ofp=os.path.join(
    result_folder,"stat_summary{}{}.csv".format(
        "_solved" if only_all_solved else "_all",
        "_grouped_an" if group_agent_num else "_separate_an"
    )
)
path_list_fp=os.path.join(result_folder,"path_file_names.csv")
time_limit=90
num_sits=6

algos=["graph"] # ["graph","exec"]
branch_orders=["default","conflict","largest_diff","random","earliest"]
use_groupings=["false","true"]
heuristics=["zero","cg_greedy","wcg_greedy"]
early_terminations=["false","true"]
w_focals=[1.0,1.1]

settings=[]
for algo in algos:
    for branch_order in branch_orders:
        for use_grouping in use_groupings:
            for heuristic in heuristics:
                for early_termination in early_terminations:
                    for w_focal in w_focals:
                        settings.append([algo,branch_order,use_grouping,heuristic,early_termination,w_focal])   

exp_headers=["map_name","agent_num","instance_idx","sit_idx"]
result_headers=["algo","branch_order","use_grouping","heuristic","w_focal","w_astar","early_termination","random_seed","status","search_time","total_time","ori_total_cost","total_cost","ori_trunc_cost","trunc_cost",
         "explored_node","pruned_node","added_node","vertex","sw_edge","heuristic_time","extra_heuristic_time","branch_time",
         "sort_time","priority_queue_time","copy_free_graphs_time","termination_time","dfs_time","grouping_time","group","group_merge_edge","group_size_max","group_size_min","group_size_avg"]
headers=exp_headers+result_headers

path_list=pd.read_csv(path_list_fp,index_col="index")
# print(path_list)

# settings=[
#     ["graph","default","false","zero","true"],
#     ["graph","largest_diff","true","wcg_greedy","true"],
# ]

data=[]
for idx in range(len(path_list)):
    map_name=path_list.iloc[idx]["map_name"]
    instance_idx=path_list.iloc[idx]["instance_idx"]
    agent_num=path_list.iloc[idx]["agent_num"]
    for sit_idx in range(num_sits):
        sit_name="map_{}_ins_{}_an_{}_sit_{}".format(map_name,instance_idx,agent_num,sit_idx)
        for setting in settings:
            algo,branch_order,use_grouping,heuristic,early_termination,w_focal=setting
            trial_name="{}_algo_{}_br_{}_gp_{}_heu_{}_et_{}_wf_{}".format(sit_name,algo,branch_order,use_grouping,heuristic,early_termination,w_focal)
            trial_stat_fp=os.path.join(stat_folder,trial_name+".json")
            datum=[map_name,agent_num,instance_idx,sit_idx]
            if not os.path.isfile(trial_stat_fp):
                if check_missing:
                    print(trial_stat_fp,"is missing")
                    exit(1)
                else:
                    continue
            with open(trial_stat_fp) as f:
                stats=json.load(f)
                for key in result_headers:
                    try:
                        datum.append(stats[key]) 
                    except KeyError as e:
                        print("key error in",trial_stat_fp)
                        import traceback
                        traceback.print_exc()
                        datum.append(None)
                        # raise e
            data.append(datum)

df=pd.DataFrame(data,columns=headers)

for column in df.columns:
    if column.find("time")!=-1:
        df[column]=df[column]/(10**6)

df["status"]=df["status"].apply(lambda x: 1 if x=="Succ" else 0)
        
if only_all_solved:
    d=df.groupby(by=["map_name","agent_num","instance_idx","sit_idx"])
    selected=d["status"].all().reset_index(name='selected')
    df=df.merge(selected,on=["map_name","agent_num","instance_idx","sit_idx"],how="inner")
    df = df[df["selected"]].drop("selected",axis=1)

df.to_csv(stats_ofp,index_label="index")

grouping_keys=["map_name","agent_num","algo","branch_order","use_grouping","heuristic","w_focal","w_astar","early_termination"]
if group_agent_num:
    grouping_keys.remove("agent_num")

summary=df.groupby(by=grouping_keys).mean().reset_index()
print(summary)
summary.to_csv(stat_summary_ofp)

print(df.groupby(by=grouping_keys)["total_time"].quantile([0.4,0.6]))