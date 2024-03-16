import pandas as pd
import os
import numpy as np
import json

result_folder="output/2024_03_16_17_13_22_exp"
if not os.path.isdir(result_folder):
    print(result_folder, "doesn't exist!")

stat_folder=os.path.join(result_folder,"stat")
stats_ofp=os.path.join(result_folder,"stats.csv")
stat_summary_ofp=os.path.join(result_folder,"stat_summary.csv")
path_list_fp=os.path.join(result_folder,"path_file_names.csv")
time_limit=90
num_sits=6
check_missing=False
algos=["graph"] # ["graph","exec"]
branch_orders=["default","conflict"]#,"largest_diff","random","earliest"]
use_groupings=["false","true"]

exp_headers=["map_name","agent_num","instance_idx","sit_idx"]
result_headers=["algo","branch_order","use_grouping","random_seed","status","search_time","total_time","ori_total_cost","total_cost","ori_trunc_cost","trunc_cost",
         "explored_node","pruned_node","added_node","vertex","sw_edge","heuristic_time","branch_time",
         "sort_time","priority_queue_time","copy_free_graphs_time","termination_time","dfs_time","grouping_time"]
headers=exp_headers+result_headers

path_list=pd.read_csv(path_list_fp,index_col="index")
# print(path_list)

data=[]
for idx in range(len(path_list)):
    map_name=path_list.iloc[idx]["map_name"]
    instance_idx=path_list.iloc[idx]["instance_idx"]
    agent_num=path_list.iloc[idx]["agent_num"]
    for sit_idx in range(num_sits):
        sit_name="map_{}_ins_{}_an_{}_sit_{}".format(map_name,instance_idx,agent_num,sit_idx)
        for algo in algos:
            for branch_order in branch_orders:
                for use_grouping in use_groupings:
                    trial_name="{}_algo_{}_br_{}_gp_{}".format(
                        sit_name,
                        algo,
                        branch_order,
                        use_grouping                                                     
                    )
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
                            datum.append(stats[key])     
                    data.append(datum)

df=pd.DataFrame(data,columns=headers)

df.to_csv(stats_ofp,index_label="index")

for column in df.columns:
    if column.find("time")!=-1:
        df[column]=df[column]/(10**6)

df["status"]=df["status"].apply(lambda x: 1 if x=="Succ" else 0)
summary=df.groupby(by=["map_name","agent_num","algo","branch_order","use_grouping"]).mean().reset_index()
print(summary)
summary.to_csv(stat_summary_ofp)

print(df.groupby(by=["map_name","agent_num","algo","branch_order","use_grouping"])["total_time"].quantile([0.4,0.6]))