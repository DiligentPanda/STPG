'''
TODO(rivers): 
We need to separate the random delay setup generation and the replanning later.
'''

import os
import time
import multiprocessing
import pandas as pd

root_folder="data/benchmark/example"
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
    stats.append((*group_name,len(group)))

stats=pd.DataFrame(stats,columns=["map_name","agent_num","instance_num"])

stats.sort_values(by=["map_name","agent_num"],inplace=True,ignore_index=True)

stats.to_csv(stats_ofp,index_label="index")
