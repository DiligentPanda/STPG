import sys
sys.path.insert(0,"analysis")
from map import Map
import matplotlib.pyplot as plt
import matplotlib.patches as patches

map_path="data/map/random-32-32-10.map"
agent_path="data/scen/scen-even/random-32-32-10-even-1.scen"
max_num_agents=None

# TODO
rect_size=1
offset=rect_size*0.5

### Draw map ###
m = Map(map_path)
rects = []
for y in range(m.height):
    for x in range(m.width):
        if m.graph[y][x] == 1:
            rect = patches.Rectangle((x*rect_size,y*rect_size),rect_size,rect_size,linewidth=1,edgecolor='black',facecolor='black')
        else:
            rect = patches.Rectangle((x,y),1,1,linewidth=1,edgecolor='black',facecolor='white')
        rects.append(rect)
        
fig, ax = plt.subplots(1,1,figsize=(10,10))

for rect in rects:
    print(rect)
    ax.add_patch(rect)

# plt.text(-0.5,0.5,"S",color="red",fontsize=10)
# plt.show()

### Draw agents ###
with open(agent_path) as f:
    f.readline() # skip the first line
    for agent_id,line in enumerate(f):
        if max_num_agents is not None and agent_id >= max_num_agents:
            break
        line = line.strip().split()
        print(line)
        start_col, start_row = int(line[4]), int(line[5])
        goal_col, goal_row = int(line[6]), int(line[7])
        # plt.scatter([start_col+offset],[start_row+offset],s=1)
        text=str(agent_id)
        plt.text(start_col,start_row+offset,text,color="red",fontsize=6)
        plt.text(goal_col+offset,goal_row+offset,text,color="blue",fontsize=6)

ax.set_xlim(0,m.width)
ax.set_ylim(0,m.height)
ax.invert_yaxis()

plt.savefig("analysis/temp/map.png", dpi=300, bbox_inches='tight')

