result_csv="output/16891_exps/stats_all.csv"
map_names = ["lak303d"]#,"lak303d"]
map_labels = ["lak303d (37-45 agents)"]#, "lak303d (37-45 agents)"]
# map_names = ["Paris_1_256"]#,"lak303d"]
# map_labels = ["Paris_1_256 (70-110 agents)"]#, "lak303d (37-45 agents)"]
output_fp="analysis/temp/16891_exps/focal_search_{}.png".format(map_labels[0])

import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

df = pd.read_csv(result_csv,index_col="index")

df = df[df["map_name"]==map_names[0]]

df = df[(df["branch_order"]=="largest_diff") & (df["use_grouping"]==True) & (df["heuristic"]=="wcg_greedy") & (df["incremental"]==True)]

# get_all_solved
d=df.groupby(by=["map_name","agent_num","instance_idx","sit_idx"])
selected=d["status"].all().reset_index(name='selected')
df=df.merge(selected,on=["map_name","agent_num","instance_idx","sit_idx"],how="inner")
df = df[df["selected"]].drop("selected",axis=1)

astar=df[df["w_focal"]==1.0]
focal=df[df["w_focal"]==1.1]

merged=pd.merge(focal,astar,on=["map_name","agent_num","instance_idx","sit_idx"],suffixes=("_focal","_astar"))

merged["solution_cost_ratio"]=((merged["ori_trunc_cost_focal"]-merged["trunc_cost_focal"])/(merged["ori_trunc_cost_focal"]-merged["trunc_cost_astar"]+1e-9))
merged["explored_node_ratio"]=(merged["explored_node_focal"]/merged["explored_node_astar"])

print(merged["map_name"].count())
print(merged[merged["explored_node_ratio"]>1.0]["map_name"].count())
print(merged[merged["explored_node_ratio"]==1.0]["map_name"].count())
print(merged[merged["explored_node_ratio"]<1.0]["map_name"].count())

print(merged["map_name"].count())
print(merged[merged["solution_cost_ratio"]>=0.25]["map_name"].count())
print(merged[merged["solution_cost_ratio"]==1.0]["map_name"].count())
print(merged[merged["solution_cost_ratio"]<0.25]["map_name"].count())

from matplotlib import pyplot as plt
plt.rcParams['font.size'] = 20
plt.figure(figsize=(10,10))
plt.scatter(merged["solution_cost_ratio"],merged["explored_node_ratio"],s=5)
plt.yscale("log")
plt.hlines(1,0.0,1.0,color='r',linestyles='dashed')
plt.title("Focal(1.1) vs A* on {}".format(map_labels[0]))
plt.xlabel("solution cost ratio")
plt.ylabel("explored node ratio")

plt.savefig(output_fp, bbox_inches='tight', dpi=300)