import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
import os


cmap=mpl.colormaps["Set1"]
ignore_num_agents = True

for prob in ["002","01","03"]:
    result_csv=f"output/baseline_comparison_0717/0717_exp_comparison_p{prob}/stats_all.csv"
    map_name = "lak303d"
    map_label = "Game"
    output_fp = f"analysis/temp/baseline_comparison_0717/success_rates_{map_label}_p{prob}.png"
    
    output_folder = os.path.split(output_fp)[0]
    os.makedirs(output_folder,exist_ok=True)

    df = pd.read_csv(result_csv,index_col="index")

    if ignore_num_agents:
        headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal"]
        algorithms={
            "CCBS": ["ccbs", "largest_diff", "all", "wcg_greedy", True, 1.0],
            "MILP": ["milp", "default","simple","zero",True,1.0],
            "GBS": ["search", "default","none","zero",False,1.0],
            "EGBS": ["search", "largest_diff","all","wcg_greedy",True,1.0],
        }  
    else:
        headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal","agent_num"]
        algorithms={
            "CCBS (41)": ["ccbs", "largest_diff", "all", "wcg_greedy", True, 1.0, 41],
            "MILP (41)": ["milp", "default","simple","zero",True,1.0,41],
            "GBS  (41)": ["search", "default","none","zero",False,1.0,41],
            "EGBS (41)": ["search", "largest_diff","all","wcg_greedy",True,1.0,41],
            "CCBS (73)": ["ccbs", "largest_diff", "all", "wcg_greedy", True, 1.0, 73],
            "MILP (73)": ["milp", "default","simple","zero",True,1.0,73],
            "GBS  (73)": ["search", "default","none","zero",False,1.0,73],
            "EGBS (73)": ["search", "largest_diff","all","wcg_greedy",True,1.0,73],
        }

    plt.rcParams.update({'font.size': 15})
    fig1, ax1=plt.subplots(figsize=(8,8))

    time_limits=[0.25,0.5,1.0,2.0,4.0,8.0,16.0]

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
        if ignore_num_agents:
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
            
        else:
            # label = "{} on {}".format(name,map_labels[map_names.index(map_name)])
            label = name
            if name.find("41")!=-1:
                color=cmap(0)
                linestyle="-"
            else:
                color=cmap(1)
                linestyle="--"
            
            markers={
                "GBS": "o",
                "CCBS": "x",
                "MILP": "d",
                "EGBS": "^",
            }
            
            marker=markers[name.split()[0]]
                
            linestyle="-"
        
        plt.plot(time_limits,success_rates,label=label,marker=marker,linestyle=linestyle,color=color)
        
    plt.xscale("log")
    plt.xticks(time_limits)    
    ax1.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
    ax1.get_xaxis().set_tick_params(which='minor', size=0)
    ax1.get_xaxis().set_tick_params(which='minor', width=0) 

    plt.legend()
    plt.title("{}".format(map_label))
    plt.xlabel("Time Limit (s)")
    plt.ylabel("Success Rate")
    plt.xlim(0,17)
    plt.ylim(-0.1,1.1)

    plt.savefig(output_fp, bbox_inches='tight', dpi=300)