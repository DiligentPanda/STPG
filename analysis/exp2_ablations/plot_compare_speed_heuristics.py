import matplotlib.pyplot as plt
import matplotlib
import matplotlib as mpl
import pandas as pd

result_csv="output/ablations_0721/2024_07_19_08_24_25_paper_exp_comparison_p01_ablation_heuristics\stats_all.csv"
output_fp = "analysis/temp/ablations_0721/compare_speed_heuristics.png"
map_names=["random-32-32-10","warehouse-10-20-10-2-1","lak303d","Paris_1_256"]  
map_labels=["Random", "Warehouse","Game","City"]
color=["blue","green","orange","purple", "brown","pink","gray","olive","cyan","magenta","yellow","black","red"]
time_limit=16

cmap=mpl.colormaps["Blues"]

df = pd.read_csv(result_csv,index_col="index")

headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal"]

algorithms=[
    ("Old Heuristics", ["search", "largest_diff", "all", "zero", True, 1.0]),
    ("New Heuristics", ["search", "largest_diff", "all", "wcg_greedy", True, 1.0])
]

plt.rcParams.update({'font.size': 25})
fig, ax=plt.subplots(figsize=(10,10))

speedups=[]
for idx,map_name in enumerate(map_names):
    df_map = df[(df["map_name"]==map_name)]

    name1, settings1=algorithms[0]
    df_alg1 = df_map.copy()
    for i in range(len(headers)):
        df_alg1= df_alg1[df_alg1[headers[i]]==settings1[i]]
    df_alg1 = df_alg1.reset_index(drop=True)
    
    name2, settings2=algorithms[1]
    df_alg2 = df_map.copy()
    for i in range(len(headers)):
        df_alg2= df_alg2[df_alg2[headers[i]]==settings2[i]]
    df_alg2 = df_alg2.reset_index(drop=True)
    
    print(df_alg1,df_alg2)
    
    ys=[]
    xs=[]
    for i in range(len(df_alg1)):
        y = df_alg1.at[i,"total_time"]
        x = df_alg2.at[i,"total_time"]
        ys.append(y)
        xs.append(x)
        
        if y<time_limit or x<time_limit:
            speedups.append(y/x)
        
    plt.scatter(xs,ys, c=color[0], alpha=0.1, s=10)
    
avg_speedup = sum(speedups)/len(speedups)
print("Average speedup of {} vs {}:".format(algorithms[1][0],algorithms[0][0]),avg_speedup)
label="{} ({:.02f})".format(algorithms[1][0],avg_speedup)
if avg_speedup>1.0:
    plt.plot([0,16/avg_speedup],[0,16],c=color[0],linestyle="--",label=label)
else:
    plt.plot([0,16],[0,16*avg_speedup],c=color[0],linestyle="--",label=label)
    

plt.xscale('log')
plt.yscale('log')
plt.xlim(0.0625/8,17)
plt.ylim(0.0625/8,17)
plt.xticks([0.01,0.06,0.25,1,4,16],["0.01","0.06","0.25","1","4","16"])
plt.xticks([],minor=True)
plt.yticks([0.01,0.06,0.25,1,4,16],["0.01","0.06","0.25","1","4","16"])
plt.yticks([],minor=True)
plt.xlabel("Other Method (s)")
plt.ylabel("Baseline Method (s)")
label="{} ({:.02f})".format(algorithms[0][0],1)
plt.plot([0,16],[0,16],c='r',linestyle="--",label=label)
# plt.plot([0,16/2.0],[0,16],c=cmap(255-32),linestyle="--",label='line y=2x')
# plt.plot([0,16/4.0],[0,16],c=cmap(255-64),linestyle="--",label='line y=4x')
# plt.plot([0,16/8.0],[0,16],c=cmap(255-96),linestyle="--",label='line y=8x')
#get handles and labels
handles, labels = plt.gca().get_legend_handles_labels()
#specify order of items in legend
order = [0,1]
#add legend to plot
plt.legend([handles[idx] for idx in order],[labels[idx] for idx in order],fontsize=20) 
plt.savefig(output_fp, bbox_inches='tight', dpi=300)

