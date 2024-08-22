import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
import os
import numpy as np


cmap=mpl.colormaps["Set1"]

for prob in ["002","01","03"]:
    
    map_name = "random-32-32-10"
    map_label = "Random"
    output_fp = f"analysis/temp/baseline_comparison_0803/execution_times_{map_label}_p{prob}.pdf"
    root_folder = f"output/exp2_p{prob}/"
    num_agents=[60,70,80,90,100]
    
    algorithms={
        "CBS-D": ["ccbs", "largest_diff", "all", "wcg_greedy", True, 1.0],
        "MILP": ["milp", "default","simple","zero",True,1.0],
        "GSES": ["search", "default","none","zero",False,1.0],
        "EGSES": ["search", "largest_diff","all","wcg_greedy",True,1.0]
    }

    output_folder = os.path.split(output_fp)[0]
    os.makedirs(output_folder,exist_ok=True)
    
    result_folders=os.listdir(root_folder)
    result_csvs=[f"{root_folder}/{folder}/stats_all.csv" for folder in result_folders]
    
    
    # result_csv=f"output/baseline_comparison_0717/0717_exp_comparison_p{prob}/stats_all.csv"

    execution_times={}
    stds={}
    for name in algorithms:
        execution_times[name]={}
        stds[name]={}
        for num_agent in num_agents:
            execution_times[name][num_agent]=[]
            stds[name][num_agent]=[]

    for result_csv in result_csvs:
        df = pd.read_csv(result_csv,index_col="index")
        headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal"]


        df1 = df[(df["map_name"]==map_name)] # & (df["agent_num"]==110)]
        for name, settings in algorithms.items():
            print("processing algorithm {}".format(name))
            df2 = df1.copy()
            for i in range(len(headers)):
                df2 = df2[df2[headers[i]]==settings[i]]
            df2 = df2.reset_index(drop=True)
            
            for num_agent in num_agents: 
                if name!="CBS-D":
                    execution_time = (df2[df2["agent_num"]==num_agent]["total_time"]+df2[df2["agent_num"]==num_agent]["grouping_time"]).mean()
                else:
                    execution_time = (df2[df2["agent_num"]==num_agent]["search_time"]).mean()
                execution_times[name][num_agent].append(execution_time)

    for name in algorithms:
        for num_agent in num_agents:
            temp=execution_times[name][num_agent]
            execution_times[name][num_agent]=np.mean(temp)
            stds[name][num_agent]=np.std(temp)*10
            print(stds[name][num_agent])
            print(f"{name} {num_agent} {execution_times[name][num_agent]} {stds[name][num_agent]}")

    plt.rcParams.update({'font.size': 35})
    fig1, ax1=plt.subplots(figsize=(8,8))

    for name in algorithms:

        label = name
        linestyle="--"
        marker="x"
        colors={
            "GSES": cmap(0),
            "CBS-D": cmap(1),
            "MILP": cmap(2),
            "EGSES": cmap(3),
        }
        color=colors[name.split()[0]]
            
        _execution_times = [execution_times[name][num_agent] for num_agent in num_agents]
        lower = [execution_times[name][num_agent]-stds[name][num_agent] for num_agent in num_agents]
        upper = [execution_times[name][num_agent]+stds[name][num_agent] for num_agent in num_agents]
        
        plt.plot(num_agents,_execution_times,label=label,marker=marker,linestyle=linestyle,color=color)
        plt.fill_between(num_agents, lower, upper, color=color, alpha=0.1)
        
    #plt.xscale("log")
    plt.xticks(num_agents)    
    ax1.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
    ax1.get_xaxis().set_tick_params(which='minor', size=0)
    ax1.get_xaxis().set_tick_params(which='minor', width=0) 

    # plt.legend()
    # plt.title("{}".format(map_label))
    plt.yticks([0,4,8,12,16])
    plt.xlabel("Number of Agents")
    plt.ylabel("Search Time (s)")
    plt.xlim(num_agents[0]*1.1-num_agents[-1]*0.1,num_agents[-1]*1.1-num_agents[0]*0.1)
    plt.ylim(-0.5,16.5)

    plt.savefig(output_fp, bbox_inches='tight', dpi=300)