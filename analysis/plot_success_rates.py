result_csv="output/16891_exps/stats_all.csv"
map_names = ["lak303d"]#,"lak303d"]
map_labels = ["lak303d"]#, "lak303d (37-45 agents)"]
# map_names = ["Paris_1_256"]#,"lak303d"]
# map_labels = ["Paris_1_256"]#, "lak303d (37-45 agents)"]
output_fp="analysis/temp/16891_exps/success_rates_{}.pdf".format(map_labels[0])

import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

df = pd.read_csv(result_csv,index_col="index")

headers=["algo","branch_order","use_grouping","heuristic","incremental","w_focal"]
algorithms={
    # "Our Best + Focal": ["graph","largest_diff",True,"wcg_greedy",True,1.1],
    "Our Best": ["graph","largest_diff",True,"wcg_greedy",True,1.0],
    "+ b,h": ["graph","largest_diff",False,"wcg_greedy",True,1.0],
    "+ d,h": ["graph","default",True,"wcg_greedy",True,1.0],
    "+ b,d": ["graph","largest_diff",True,"zero",True,1.0],
    "+ d": ["graph","default",True,"zero",True,1.0],
    "+ b": ["graph","largest_diff",False,"zero",True,1.0],
    "+ h": ["graph","default",False,"wcg_greedy",True,1.0],
    "Baseline": ["graph","default",False,"zero",True,1.0],
}

plt.rcParams.update({'font.size': 20})
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
        plt.plot(time_limits,success_rates,label=label,marker="o",linestyle="-")
    
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