import pandas as pd
import json
import numpy as np

data_folder="data/benchmark/example"
stats_fp="output/2024_03_31_22_39_38_exp/stats_all.csv"

stats=pd.read_csv(stats_fp)

print(stats)

data=[]
for i in range(len(stats)):
    row=stats.iloc[i]
    print(row["map_name"])
    print(row["agent_num"])
    print(row["instance_idx"])
    print(row["status"])
    
    # interested now: #delayed agents, #total delays, mean init state, #expanded nodes
    
    sit_fp="{}/sit/map_{}_ins_{}_an_{}_sit_{}.json".format(data_folder,row["map_name"],row["instance_idx"],row["agent_num"],row["sit_idx"])
    with open(sit_fp) as f:
        sit_data=json.load(f)
    num_delayed_agents=np.count_nonzero(sit_data["delay_steps"])
    total_delays=np.sum(sit_data["delay_steps"])
    max_delays=np.max(sit_data["delay_steps"])
    print(num_delayed_agents,total_delays,max_delays)
    datum={}
    for j in row.keys():
        datum[j]=row[j]
    datum["num_delayed_agents"]=num_delayed_agents
    datum["total_delays"]=total_delays
    datum["max_delays"]=max_delays
    print(datum)

# folder="example/sit"

# import os
# import json
# import numpy as np

# for fn in os.listdir(folder):
#     fp=os.path.join(folder,fn)
#     with open(fp,'r') as f:
#         data=json.load(f)
#         if np.count_nonzero(data["delay_steps"])>1:
#             print(fn,np.count_nonzero(data["delay_steps"]))
            
            
