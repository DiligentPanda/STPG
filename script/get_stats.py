import pandas as pd
import os
import numpy as np
import json
import argparse
import subprocess

arg_parser=argparse.ArgumentParser()
arg_parser.add_argument("-f","--folder",type=str,default="")
arg_parser.add_argument("-c","--check_missing",action="store_true")
arg_parser.add_argument("-s","--only_all_solved",action="store_true")
arg_parser.add_argument("-g","--group_agent_num",action="store_true")
arg_parser.add_argument("-a","--all",action="store_true")

args=arg_parser.parse_args()

if args.all:
    output_folder=args.folder
    subprocess.check_output(f"python script/get_stats.py -f {output_folder}", shell=True) 
    subprocess.check_output(f"python script/get_stats.py -f {output_folder} -s", shell=True) 
    subprocess.check_output(f"python script/get_stats.py -f {output_folder} -g", shell=True) 
    subprocess.check_output(f"python script/get_stats.py -f {output_folder} -s -g", shell=True) 
    exit(0)


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
num_sits=6

# algos=["search"] # ["search"]
# branch_orders=["largest_diff","default"] #,"random","earliest"]
# grouping_methods=["simple","all"]
# heuristics=["wcg_greedy","zero"]
# early_terminations=["true"]
# incrementals=["true"]
# w_focals=[1.0]

# settings=[]
# for algo in algos:
#     for branch_order in branch_orders:
#         for use_grouping in grouping_methods:
#             for heuristic in heuristics:
#                 for early_termination in early_terminations:
#                     for incremental in incrementals:
#                         for w_focal in w_focals:
#                             settings.append([algo,branch_order,use_grouping,heuristic,early_termination,incremental,w_focal])   


exp_headers=["map_name","agent_num","instance_idx","sit_idx"]
result_headers=["algo","branch_order","grouping_method","heuristic","w_focal","w_astar","early_termination","incremental","random_seed","status","search_time","total_time","original_cost","cost",
         "explored_node","pruned_node","added_node","vertex","sw_edge","heuristic_time","extra_heuristic_time","branch_time",
         "sort_time","priority_queue_time","copy_free_graphs_time","termination_time","dfs_time","grouping_time","group","group_merge_edge","group_size_max","group_size_min","group_size_avg"]
headers=exp_headers+result_headers

path_list=pd.read_csv(path_list_fp,index_col="index")
# print(path_list)

milp_setting2=["milp","default","simple","zero","true","true",1.0]           
# milp_setting1=["milp","default","all","zero","true","true",1.0]        
#old_setting=["search","default","simple","zero","true","true",1.0]
new_setting=["search","largest_diff","all","wcg_greedy","true","true",1.0]

oldest_setting1=["search","default","none","zero","true","false",1.0]
# oldest_setting2=["search","default","none","zero","true","true",1.0]
# oldest_setting3=["search","default","simple","zero","true","false",1.0]

ccbs_setting=["ccbs","largest_diff","all","wcg_greedy","true","true",1.0]
        
settings=[oldest_setting1,new_setting,milp_setting2,ccbs_setting]

# oldest_setting1=["search","default","none","zero","true","false",1.0]
# oldest_setting2=["search","default","none","zero","true","true",1.0]
# oldest_setting3=["search","default","simple","zero","true","false",1.0]
        
# oldest_settings=[oldest_setting1,oldest_setting2,oldest_setting3]


# settings=[]
# for algo in algos:
#     for branch_order in branch_orders:
#         for grouping_method in grouping_methods:
#             for heuristic in heuristics:
#                 for early_termination in early_terminations:
#                     for incremental in incrementals:
#                         for w_focal in w_focals:
#                             settings.append([algo,branch_order,grouping_method,heuristic,early_termination,incremental,w_focal])   
                            
data=[]
for idx in range(len(path_list)):
    map_name=path_list.iloc[idx]["map_name"]
    instance_idx=path_list.iloc[idx]["instance_idx"]
    agent_num=path_list.iloc[idx]["agent_num"]
    for sit_idx in range(num_sits):
        sit_name="map_{}_ins_{}_an_{}_sit_{}".format(map_name,instance_idx,agent_num,sit_idx)
        for setting in settings:
            algo,branch_order,use_grouping,heuristic,early_termination,incremental,w_focal=setting
            trial_name="{}_algo_{}_br_{}_gp_{}_heu_{}_et_{}_inc_{}_wf_{}".format(sit_name,algo,branch_order,use_grouping,heuristic,early_termination,incremental,w_focal)
            trial_stat_fp=os.path.join(stat_folder,trial_name+".json")
            datum=[map_name,agent_num,instance_idx,sit_idx]
            if not os.path.isfile(trial_stat_fp):
                if check_missing:
                    print(trial_stat_fp,"is missing")
                    exit(1)
                else:
                    continue
            with open(trial_stat_fp) as f:
                try:
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
                except:
                    print("Error loading",trial_stat_fp)
                    import traceback
                    traceback.print_exc()
                    continue
            data.append(datum)

df=pd.DataFrame(data,columns=headers)

print(df.head(10))

for column in df.columns:
    if column.find("time")!=-1:
        df[column]=df[column]/(10**6)

df["status"]=df["status"].apply(lambda x: 1 if x=="Succ" else 0)
        
if only_all_solved:
    d=df.groupby(by=["map_name","agent_num","instance_idx","sit_idx"])
    selected=(d["status"].sum()==len(settings)).reset_index(name='selected')
    df=df.merge(selected,on=["map_name","agent_num","instance_idx","sit_idx"],how="inner")
    df = df[df["selected"]].drop("selected",axis=1)

df.to_csv(stats_ofp,index_label="index")

grouping_keys=["map_name","agent_num","algo","branch_order","grouping_method","heuristic","w_focal","w_astar","early_termination","incremental"]
if group_agent_num:
    grouping_keys.remove("agent_num")

summary=df.groupby(by=grouping_keys).mean().reset_index()
# print(summary)
summary.to_csv(stat_summary_ofp)

print(df.groupby(by=grouping_keys)["total_time"].quantile([0.4,0.6]))