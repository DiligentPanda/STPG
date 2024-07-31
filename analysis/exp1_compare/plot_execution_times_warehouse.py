import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
import os


cmap=mpl.colormaps["Set1"]

for prob in ["002","01","03"]:
    result_csv=f"output/baseline_comparison_0717/0717_exp_comparison_p{prob}/stats_all.csv"
    map_name = "warehouse-10-20-10-2-1"
    map_label = "Warehouse"
    output_fp = f"analysis/temp/baseline_comparison_0717/execution_times_{map_label}_p{prob}.pdf"
    
    output_folder = os.path.split(output_fp)[0]
    os.makedirs(output_folder,exist_ok=True)

    df = pd.read_csv(result_csv,index_col="index")
    headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal"]
    algorithms={
        "CCBS": ["ccbs", "largest_diff", "all", "wcg_greedy", True, 1.0],
        "MILP": ["milp", "default","simple","zero",True,1.0],
        "GBS": ["search", "default","none","zero",False,1.0],
        "EGBS": ["search", "largest_diff","all","wcg_greedy",True,1.0],
    }  


    plt.rcParams.update({'font.size': 15})
    fig1, ax1=plt.subplots(figsize=(8,8))

    num_agents=[110,120,130,140,150]

    df1 = df[(df["map_name"]==map_name)] # & (df["agent_num"]==110)]
    execution_times={}
    lower_quantiles={}
    upper_quantiles={}
    for name, settings in algorithms.items():
        print("processing algorithm {}".format(name))
        df2 = df1.copy()
        for i in range(len(headers)):
            df2 = df2[df2[headers[i]]==settings[i]]
        df2 = df2.reset_index(drop=True)
        
        execution_times[name] = []
        lower_quantiles[name] = []
        upper_quantiles[name] = []
        for num_agent in num_agents: 
            execution_time = df2[df2["agent_num"]==num_agent]["total_time"].mean()
            lower_quantiles[name].append(df2[df2["agent_num"]==num_agent]["total_time"].quantile(0.4))
            upper_quantiles[name].append(df2[df2["agent_num"]==num_agent]["total_time"].quantile(0.6))
            execution_times[name].append(execution_time)

    for name, _execution_times in execution_times.items():

        label = name
        linestyle="--"
        marker="x"
        colors={
            "GBS": cmap(0),
            "CCBS": cmap(1),
            "MILP": cmap(2),
            "EGBS": cmap(3),
        }
        color=colors[name.split()[0]]
            
        plt.plot(num_agents,_execution_times,label=label,marker=marker,linestyle=linestyle,color=color)
        plt.fill_between(num_agents, lower_quantiles[name], upper_quantiles[name], color=color, alpha=0.1)
        
    #plt.xscale("log")
    plt.xticks(num_agents)    
    ax1.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
    ax1.get_xaxis().set_tick_params(which='minor', size=0)
    ax1.get_xaxis().set_tick_params(which='minor', width=0) 

    plt.legend()
    plt.title("{}".format(map_label))
    plt.xlabel("Number of Agents")
    plt.ylabel("Optimization Time (s)")
    plt.xlim(num_agents[0]*1.1-num_agents[-1]*0.1,num_agents[-1]*1.1-num_agents[0]*0.1)
    plt.ylim(-0.5,16.5)

    plt.savefig(output_fp, bbox_inches='tight', dpi=300)