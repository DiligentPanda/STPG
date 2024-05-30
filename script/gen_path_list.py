'''
TODO(rivers): 
We need to separate the random delay setup generation and the replanning later.
'''

import os
import time
import multiprocessing
import pandas as pd
import numpy as np

root_folder="data/benchmark/test_PBS2_sadg"
path_folder=os.path.join(root_folder,"path")
file_names_ofp=os.path.join(root_folder,"path_file_names.csv")
stats_ofp=os.path.join(root_folder,"path_stats.csv")

data=[]
for path_file_name in os.listdir(path_folder):
    name=os.path.splitext(path_file_name)[0]
    idx_map=name.find("map_")
    idx_ins=name.find("ins_")
    idx_an=name.find("an_")
    map_name=name[idx_map+4:idx_ins-1]
    instance_idx=int(name[idx_ins+4:idx_an-1])
    agent_num=int(name[idx_an+3:])
    data.append((path_file_name,map_name,instance_idx,agent_num))
    
df=pd.DataFrame(data,columns=["file_path","map_name","instance_idx","agent_num"])

df.sort_values(by=["map_name","agent_num","instance_idx"],inplace=True,ignore_index=True)

df.to_csv(file_names_ofp,index_label="index")


groups=df.groupby(by=["map_name","agent_num"])

stats=[]
for group_name,group in groups:
    stats.append([*group_name,len(group)])

map_agent_nums={}
for stat in stats:
    map_name=stat[0]
    if map_name not in map_agent_nums:
        map_agent_nums[map_name]=[]
        for instance in range(0,25):
            with open("data/scen/sadg_scen/"+map_name+"-"+str(instance+1)+".scen") as f:
                map_agent_nums[map_name].append(len(f.readlines())-1)
        map_agent_nums[map_name]=np.array(map_agent_nums[map_name])

for stat in stats:
    map_name=stat[0]
    agent_num=int(stat[1])
    solved_instance_num=stat[2]
    total_instance_num=np.count_nonzero(map_agent_nums[map_name]>=agent_num)
    stat.append(total_instance_num)
    stat.append(solved_instance_num/total_instance_num if total_instance_num>0 else -1)

stats=pd.DataFrame(stats,columns=["map_name","agent_num","solved_instance_num","total_instance_num","success_rate"])

stats.sort_values(by=["map_name","agent_num"],inplace=True,ignore_index=True)

stats.to_csv(stats_ofp,index_label="index")
