import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

result_csv="output/2024_07_17_07_54_42_paper_exp_comparison_p01/stats_all.csv"
map_names=["random-32-32-10","warehouse-10-20-10-2-1","lak303d","Paris_1_256"]  
map_labels=["random", "warehouse","game","city"]
color=["blue","red","green","purple","orange","brown","pink","gray","olive","cyan","magenta","yellow","black"]

df = pd.read_csv(result_csv,index_col="index")

headers=["algo","branch_order","grouping_method","heuristic","incremental","w_focal"]

algorithms=[
    ("EGBS", ["search", "largest_diff", "all", "wcg_greedy", True, 1.0]),
    ("GBS", ["search", "default", "none", "zero", False, 1.0])
]

plt.rcParams.update({'font.size': 25})
fig, ax=plt.subplots(figsize=(10,10))

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
        ys.append(df_alg1.at[i,"total_time"])
        xs.append(df_alg2.at[i,"total_time"])
        
    plt.scatter(xs,ys,label=map_labels[idx], c=color[idx], alpha=0.5, s=10)

# plt.xscale('log')
# plt.yscale('log')
plt.xlim(-1,17)
plt.ylim(-1,17)
plt.xticks([0,4,8,12,16])
plt.yticks([0,4,8,12,16])
plt.xlabel(algorithms[1][0]+" (s)")
plt.ylabel(algorithms[0][0]+" (s)")
plt.plot([-0.5,16.5],[-0.5,16.5],c="black",linestyle="--",label='line y=x')
plt.legend(markerscale=3)
plt.savefig("analysis/temp/compare_speed.png")

