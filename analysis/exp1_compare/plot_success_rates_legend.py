import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
import os
import numpy as np

cmap=mpl.colormaps["Set1"]
ignore_num_agents = True

for prob in ["01"]:
    map_name = "warehouse-10-20-10-2-1"
    map_label = "Warehouse"
    output_fp = f"analysis/temp/baseline_comparison_0803/success_rates_legend.pdf"
    root_folder = f"output/exp2_p{prob}/"
    
    time_limits=[0.5,1.0,2.0,4.0,8.0,16.0]
    
    result_folders=os.listdir(root_folder)
    result_csvs=[f"{root_folder}/{folder}/stats_all.csv" for folder in result_folders]
    
    output_folder = os.path.split(output_fp)[0]
    os.makedirs(output_folder,exist_ok=True)

    if ignore_num_agents:
        headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal"]
        algorithms={
            "CBS-D": ["ccbs", "largest_diff", "all", "wcg_greedy", True, 1.0],
            "MILP": ["milp", "default","simple","zero",True,1.0],
            "GSES": ["search", "default","none","zero",False,1.0],
            "IGSES": ["search", "largest_diff","all","wcg_greedy",True,1.0],
        }  
    else:
        headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal","agent_num"]
        algorithms={
            "CBS-D (110)": ["ccbs", "largest_diff", "all", "wcg_greedy", True, 1.0, 110],
            "MILP (110)": ["milp", "default","simple","zero",True,1.0,110],
            "GSES  (110)": ["search", "default","none","zero",False,1.0,110],
            "IGSES (110)": ["search", "largest_diff","all","wcg_greedy",True,1.0,110],
            "CBS-D (150)": ["ccbs", "largest_diff", "all", "wcg_greedy", True, 1.0, 150],
            "MILP (150)": ["milp", "default","simple","zero",True,1.0,150],
            "GSES  (150)": ["search", "default","none","zero",False,1.0,150],
            "IGSES (150)": ["search", "largest_diff","all","wcg_greedy",True,1.0,150],
        }

    plt.rcParams.update({'font.size': 35})
    
    figlegend = plt.figure(figsize=(16,1))
    fig1, ax1=plt.subplots(figsize=(8,8))

    success_rates={}
    stds={}
    for name in algorithms:
        success_rates[name]={}
        stds[name]={}
        for time_limit in time_limits:
            success_rates[name][time_limit]=[]
            stds[name][time_limit]=[]

    for result_csv in result_csvs:

        df = pd.read_csv(result_csv,index_col="index")

        df1 = df[(df["map_name"]==map_name)] # & (df["agent_num"]==110)]
        for name, settings in algorithms.items():
            print("processing algorithm {}".format(name))
            df2 = df1.copy()
            for i in range(len(headers)):
                df2 = df2[df2[headers[i]]==settings[i]]
            df2 = df2.reset_index(drop=True)
            
            for time_limit in time_limits:
                t=df2["total_time"]
                if name!="CBS-D":
                    t=+df2["grouping_time"]
                success_rate = ((df2["status"]>0) & (t<=time_limit)).mean()
                success_rates[name][time_limit].append(success_rate)


    for name in algorithms:
        for time_limit in time_limits:
            temp=success_rates[name][time_limit]
            success_rates[name][time_limit]=np.mean(temp)
            stds[name][time_limit]=np.std(temp)*10
            print(f"{name} {time_limit} {success_rates[name][time_limit]} {stds[name][time_limit]}")

    for name in algorithms:
        if ignore_num_agents:
            label = name
            linestyle="--"
            marker="x"
            colors={
                "GSES": cmap(0),
                "CBS-D": cmap(1),
                "MILP": cmap(2),
                "IGSES": cmap(3),
            }
            color=colors[name.split()[0]]
            
        else:
            # label = "{} on {}".format(name,map_labels[map_names.index(map_name)])
            label = name
            if name.find("120")!=-1:
                color=cmap(0)
                linestyle="-"
            else:
                color=cmap(1)
                linestyle="--"
            
            markers={
                "GSES": "o",
                "CBS-D": "x",
                "MILP": "d",
                "IGSES": "^",
            }
        
            marker=markers[name.split()[0]]
        
        _success_rates = [success_rates[name][time_limit] for time_limit in time_limits]
        lower = [success_rates[name][time_limit]-stds[name][time_limit] for time_limit in time_limits]
        upper = [success_rates[name][time_limit]+stds[name][time_limit] for time_limit in time_limits]
        
        plt.plot(time_limits,_success_rates,label=label,marker=marker,linestyle=linestyle,color=color)
        plt.fill_between(time_limits, lower, upper, color=color, alpha=0.1)
        
    figlegend.legend(ax1.get_legend_handles_labels()[0], ax1.get_legend_handles_labels()[1], loc='center', ncol=4)
        
    plt.xscale("log")
    plt.xticks(time_limits)    
    ax1.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
    ax1.get_xaxis().set_tick_params(which='minor', size=0)
    ax1.get_xaxis().set_tick_params(which='minor', width=0) 

    # plt.legend()
    # plt.title("{}".format(map_label))
    plt.xlabel("Time Limit (s)")
    plt.ylabel("Success Rate")
    plt.xlim(0,17)
    plt.ylim(-0.1,1.1)

    # plt.savefig(output_fp, bbox_inches='tight', dpi=300)
    figlegend.savefig(output_fp, bbox_inches='tight', dpi=300)
    # plt.show()