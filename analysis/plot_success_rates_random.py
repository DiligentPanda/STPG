result_csv="output/2024_05_09_06_53_48_exp/stats_all.csv"
# map_names = ["lak303d"]#,"lak303d"]
# map_labels = ["lak303d"]#, "lak303d (37-45 agents)"]
map_names = ["random-32-32-10"]#,"lak303d"]
map_labels = ["random-32-32-10"]#, "lak303d (37-45 agents)"]
output_fp="analysis/temp/milp_compare/success_rates_{}.png".format(map_labels[0])

import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

df = pd.read_csv(result_csv,index_col="index")

headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal","agent_num"]
algorithms={
    "milp + simple g (70)": ["milp", "default","simple","zero",True,1.0,70],
    "milp + all g (70)": ["milp", "default","all","zero",True,1.0,70],
    "search default* (70)": ["search", "default","simple","zero",True,1.0,70],
    "search best (70)": ["search", "largest_diff","all","wcg_greedy",True,1.0,70],
    "milp + simple g (85)": ["milp", "default","simple","zero",True,1.0,85],
    "milp + all g (85)": ["milp", "default","all","zero",True,1.0,85],
    "search default* (85)": ["search", "default","simple","zero",True,1.0,85],
    "search best (85)": ["search", "largest_diff","all","wcg_greedy",True,1.0,85],
    "milp + simple g (100)": ["milp", "default","simple","zero",True,1.0,100],
    "milp + all g (100)": ["milp", "default","all","zero",True,1.0,100],
    "search default* (100)": ["search", "default","simple","zero",True,1.0,100],
    "search best (100)": ["search", "largest_diff","all","wcg_greedy",True,1.0,100],
}

plt.rcParams.update({'font.size': 15})
fig1, ax1=plt.subplots(figsize=(10,6))

time_limits=[0.1,0.2,0.5,1.0,2.0,5.0,10.0,20.0,50.0,90.0]

for map_name in map_names:
    df1 = df[(df["map_name"]==map_name)] # & (df["agent_num"]==110)]
    success_rates={}
    for name, settings in algorithms.items():
        print("processing algorithm {}".format(name))
        df2 = df1.copy()
        for i in range(len(headers)):
            df2 = df2[df2[headers[i]]==settings[i]]
        df2 = df2.reset_index(drop=True)
        
        success_rates[name] = []
        for time_limit in time_limits:
            success_rate = ((df2["status"]>0) & (df2["search_time"]<=time_limit)).mean()
            success_rates[name].append(success_rate)

    print(success_rates)

    for name, success_rates in success_rates.items():
        # label = "{} on {}".format(name,map_labels[map_names.index(map_name)])
        label = name
        if name.find("milp")!=-1:
            marker='x'
        else:
            marker="o"
            
        if name.find("simple")!=-1 or name.find("default")!=-1:
            linestyle='--'
        else:
            linestyle="-"
        
        if name.find("70")!=-1:
            color='r'
        elif name.find("85")!=-1:
            color='g'
        else:
            color='b'
        
        plt.plot(time_limits,success_rates,label=label,marker=marker,linestyle=linestyle,color=color)
    
plt.xscale("log")
plt.xticks(time_limits)    
ax1.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
ax1.get_xaxis().set_tick_params(which='minor', size=0)
ax1.get_xaxis().set_tick_params(which='minor', width=0) 

plt.legend()
plt.title("{}".format(map_labels[0]))
plt.xlabel("Time Limit (s)")
plt.ylabel("Success Rate")

plt.savefig(output_fp, bbox_inches='tight', dpi=300)