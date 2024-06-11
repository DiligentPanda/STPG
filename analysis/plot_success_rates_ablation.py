import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

result_csv="output/2024_06_01_09_44_08_exp/stats_all.csv"
# map_names = ["lak303d"]#,"lak303d"]
# map_labels = ["lak303d"]#, "lak303d (37-45 agents)"]
map_names = ["random-32-32-10","warehouse-10-20-10-2-1","Paris_1_256","lak303d"]#,"lak303d"]
map_labels = ["random","warehouse","city","game"]#, "lak303d (37-45 agents)"]

for map_name, map_label in zip(map_names, map_labels):
    output_fp="analysis/temp/milp_compare/success_rates_{}_abalation.png".format(map_label)

    df = pd.read_csv(result_csv,index_col="index")

    headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal"]
    algorithms={
        "b_l + g_a + h_w": ["search", "largest_diff", "all", "wcg_greedy", True, 1.0], 
        "b_l + g_a + h_z": ["search", "largest_diff", "all", "zero", True, 1.0], 
        "b_l + g_s + h_w": ["search", "largest_diff", "simple", "wcg_greedy", True, 1.0], 
        "b_l + g_s + h_z": ["search", "largest_diff", "simple", "zero", True, 1.0],  
        "b_d + g_a + h_w": ["search", "default", "all", "wcg_greedy", True, 1.0], 
        "b_d + g_a + h_z": ["search", "default", "all", "zero", True, 1.0], 
        "b_d + g_s + h_w": ["search", "default", "simple", "wcg_greedy", True, 1.0], 
        "b_d + g_s + h_z": ["search", "default", "simple", "zero", True, 1.0],  
    }

    # algorithms = pd.DataFrame.from_dict(algorithms, orient='index', columns=headers)



    plt.rcParams.update({'font.size': 12})
    fig1, ax1=plt.subplots(figsize=(10,6))

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

    print("success_rates:", success_rates)

    lengended=set()
    for name, success_rates in success_rates.items():
        # label = "{} on {}".format(name,map_labels[map_names.index(map_name)])
        label = name
        settings = algorithms[name]
        
        if name.find("b_l")!=-1:
            linestyle='-'
        else:
            linestyle="--"
        
        if name.find("g_a")!=-1:
            marker='o'
        else:
            marker="x"
            
        if name.find("h_w")!=-1:
            color='r'
        else:
            color="b"
        
        plt.plot(time_limits,success_rates,marker=marker,linestyle=linestyle,color=color,label=label)

    import matplotlib.lines as mlines
    from matplotlib.legend_handler import HandlerLine2D, HandlerTuple

    dummy = mlines.Line2D([], [], color='None', marker='None', linestyle='None',markersize=1)
    # red0 = mlines.Line2D([], [], color='r', marker='o', linestyle='None')
    # blue1 = mlines.Line2D([], [], color='b', marker='x', linestyle='None')
    # red1 = mlines.Line2D([], [], color='r', marker='x', linestyle='None')
    # blue2 = mlines.Line2D([], [], color='b', marker='None', linestyle='-')
    # red2 = mlines.Line2D([], [], color='r', marker='None', linestyle='-')
    # blue3 = mlines.Line2D([], [], color='b', marker='None', linestyle='--')
    # red3 = mlines.Line2D([], [], color='r', marker='None', linestyle='--')

    plt.legend(handles=[dummy]*6, labels=[
        "red: h_w",
        "blue: h_z",
        "circle: g_a",
        "cross: g_s",
        'solid: b_l',
        'dashed: b_d'
        ], handlelength=0, handletextpad=0)

    #plt.legend()


    plt.xscale("log")
    plt.xticks(time_limits)    
    ax1.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    ax1.get_xaxis().set_tick_params(which='minor', size=0)
    ax1.get_xaxis().set_tick_params(which='minor', width=0) 

    plt.title("{}".format(map_labels[0]))
    plt.xlabel("Time Limit (s)")
    plt.ylabel("Success Rate")

    plt.savefig(output_fp, bbox_inches='tight', dpi=300)