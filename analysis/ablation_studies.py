result_csv="output/16891_exps/stats_all.csv"

import pandas as pd

df = pd.read_csv(result_csv,index_col="index")

# remove focal search
df = df[df["w_focal"]==1.0]

# get_all_solved
d=df.groupby(by=["map_name","agent_num","instance_idx","sit_idx"])
selected=d["status"].all().reset_index(name='selected')
df=df.merge(selected,on=["map_name","agent_num","instance_idx","sit_idx"],how="inner")
df = df[df["selected"]].drop("selected",axis=1)

# baseline=df[(df["algo"]=="graph") & (df["branch_order"]=="default") & (df["use_grouping"]==False) & (df["heuristic"]=="zero") & (df["incremental"]==False)]

# df=df.merge(baseline,on=["map_name","agent_num","instance_idx","sit_idx"],how="inner",suffixes=("","_baseline"))

# df["search_time"]=df["search_time"]/df["search_time_baseline"]
# df["explored_node"]=df["explored_node"]/df["explored_node_baseline"]
# df["added_node"]=df["added_node"]/df["added_node_baseline"]

# df.to_csv("temp.csv")

grouping_keys=["map_name","algo","branch_order","use_grouping","heuristic","incremental"]

summary=df.groupby(by=grouping_keys).mean().reset_index()
summary=summary[grouping_keys+["search_time","explored_node","added_node"]]

print(summary[(summary["incremental"]==False) & (summary["map_name"]=="lak303d")])


print(df["status"].mean())