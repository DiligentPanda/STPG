import pandas as pd
import os
import numpy as np

result_folder="output/2024_03_03_09_09_38"
main_result_folder=os.path.join(result_folder,"main")
stats_ofp=os.path.join(result_folder,"stats.csv")
time_limit=90

exp_headers=["algo_name","map_name","agent_num","instance_idx","trial_idx"]
result_headers=["search_time","total_time","ori_total_cost","ori_trunc_cost","total_cost","trunc_cost",
         "explored_node","pruned_node","added_node","vertex","sw_edge","heuristic_time","branch_time",
         "sort_time","priority_queue_time","copy_free_graph_time","termination_time","dfs_time"]
headers=exp_headers+result_headers

data=[]
for file_name in os.listdir(main_result_folder):
    file_path=os.path.join(main_result_folder,file_name)
    name,ext=os.path.splitext(file_name)
    algo_name=ext[1:]
    idx_map=name.find("map_")
    idx_ins=name.find("ins_")
    idx_an=name.find("an_")
    idx_tr=name.find("tr_")
    map_name=name[idx_map+4:idx_ins-1]
    instance_idx=int(name[idx_ins+4:idx_an-1])
    agent_num=int(name[idx_an+3:idx_tr-1])
    trial_idx=int(name[idx_tr+3:])
    datum=[algo_name,map_name,agent_num,instance_idx,trial_idx]
    with open(file_path) as f:
        line=f.readline()
        records=line.strip().split(",")
        records=[int(v) if len(v)>0 else np.nan for v in records]
        datum+=records
    data.append(datum)
    
df=pd.DataFrame(data,columns=headers)

df.to_csv(stats_ofp,index="index")

for column in df.columns:
    if column.find("time")!=-1:
        df[column]=df[column]/(10**6)

print(df.groupby(by=["algo_name","map_name","agent_num"]).mean())
        