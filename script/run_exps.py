import os
import time
import multiprocessing
import pandas as pd
import subprocess

exe_path="./build/simulate"
root_folder="data/benchmark/test_PBS2_delay_p002"
path_folder=os.path.join(root_folder,"path")
sit_folder=os.path.join(root_folder,"sit")
file_names_fp=os.path.join(root_folder,"path_file_names.csv")
exp_desc="paper_exp_comparison_p002" # describe the experiments

timestamp=time.strftime("%Y_%m_%d_%H_%M_%S")
exp_name="{}_{}".format(timestamp,exp_desc)
# TODO(rivers): merge output into one or separate them somehow?
output_folder="output/{}".format(exp_name)
path_list_ofp=os.path.join(output_folder,"path_file_names.csv")
# the folllowing three are actually useless, because we will generate delay scenarios separately
# delay_prob=10
# delay_steps_low=10
# delay_steps_high=20
time_limit=16
# algos=["search"] # ["search"]
# branch_orders=["largest_diff","default"] #,"random","earliest"]
# grouping_methods=["simple","all"]
# heuristics=["wcg_greedy","zero"]
# early_terminations=["true"]
# incrementals=["true","false"]
# w_focals=[1.0]
MAX_VIRTUAL_MEMORY = 16 * 1024 * 1024 # 8 GB
skip=False

instance_idxs=list(range(1,25+1)) # this is the number provided by the benchmark
num_sits=6

subprocess.check_output("./compile.sh", shell=True) 

# setting: [agent_num_start, agent_num_end, agent_num_step, max_process_num]
maps = {
        "random-32-32-10": [60,100,10,8],
        "warehouse-10-20-10-2-1": [110,150,10,8],
        "Paris_1_256": [120,200,20,8],
        "lak303d": [41,73,8,8]
}

stat_output_folder=os.path.join(output_folder,"stat")
fail_output_folder=os.path.join(output_folder,"fail")
new_path_output_folder=os.path.join(output_folder,"path")

os.makedirs(stat_output_folder,exist_ok=True)
os.makedirs(fail_output_folder,exist_ok=True)
os.makedirs(new_path_output_folder,exist_ok=True)

df=pd.read_csv(file_names_fp,index_col="index")
df.sort_values(by=["map_name","agent_num","instance_idx"],inplace=True,ignore_index=True)

# filter
def keep(row):
    map_name = row["map_name"]
    agent_num = row["agent_num"]
    instance_idx = row["instance_idx"]
    if map_name not in maps:
        return False
    
    setting = maps[map_name]
    agent_num_start,agent_num_end,agent_num_step,max_process_num=setting
    
    if agent_num not in range(agent_num_start,agent_num_end+agent_num_step,agent_num_step):
        return False
    
    if instance_idx not in instance_idxs:
        return False
    
    return True

df = df[df.apply(keep,axis=1)]
df = df.reset_index(drop=True)
df.to_csv(path_list_ofp,index_label="index")

def log_fail(output_name,result=None,exception=None):
    fail_output_file_path=os.path.join(fail_output_folder,output_name+".fail")
    with open(fail_output_file_path,'w') as f:
        if result is not None:
            f.write("stdout: \n")
            f.write(result.stdout)
            f.write("\n")
            f.write("stderr: \n")
            f.write(result.stderr)
        if exception is not None:
            f.write("exception: \n")
            f.write(exception)

def run(cmd,output_name):
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print("[SUCC] {}".format(cmd))
            print(result.stdout)
        else:
            print("[FAIL] {}".format(cmd))
            print(result.stdout)
            print(result.stderr)
            log_fail(output_name,result=result)
    except Exception:
        import traceback
        print("[EXCEPTION]", traceback.format_exc())
        log_fail(output_name,exception=traceback.format_exc())

milp_setting2=["milp","default","simple","zero","true","true",1.0]           
# milp_setting1=["milp","default","all","zero","true","true",1.0]        
#old_setting=["search","default","simple","zero","true","true",1.0]
new_setting=["search","largest_diff","all","wcg_greedy","true","true",1.0]

oldest_setting1=["search","default","none","zero","true","false",1.0]
# oldest_setting2=["search","default","none","zero","true","true",1.0]
# oldest_setting3=["search","default","simple","zero","true","false",1.0]
        
settings=[oldest_setting1,new_setting,milp_setting2]
#settings=[old_setting,new_setting,milp_setting1,milp_setting2]
# settings=[]
# for algo in algos:
#     for branch_order in branch_orders:
#         for grouping_method in grouping_methods:
#             for heuristic in heuristics:
#                 for early_termination in early_terminations:
#                     for incremental in incrementals:
#                         for w_focal in w_focals:
#                             settings.append([algo,branch_order,grouping_method,heuristic,early_termination,incremental,w_focal])   

for map_name,setting in maps.items():
    print("processing map {}".format(map_name))
    agent_num_start,agent_num_end,agent_num_step,max_process_num=setting
    
    pool=multiprocessing.Pool(max_process_num)
    
    exps=df[df["map_name"]==map_name].values.tolist()
    
    cmds=[]
    output_names=[]
    for exp in exps:
        path_file_name,map_name,instance_idx,agent_num=exp
        path_file_path=os.path.join(path_folder,path_file_name)
        for sit_idx in range(num_sits):
            sit_name="map_{}_ins_{}_an_{}_sit_{}".format(map_name,instance_idx,agent_num,sit_idx)
            sit_file_path=os.path.join(sit_folder,sit_name+".json")
            for setting in settings:
                algo,branch_order,grouping_method,heuristic,early_termination,incremental,w_focal=setting
                trial_name="{}_algo_{}_br_{}_gp_{}_heu_{}_et_{}_inc_{}_wf_{}".format(sit_name,algo,branch_order,grouping_method,heuristic,early_termination,incremental,w_focal)
                output_names.append(trial_name)
                stat_ofp=os.path.join(stat_output_folder,trial_name+".json")
                new_path_ofp=os.path.join(new_path_output_folder,trial_name+".path")
                
                if skip and os.path.exists(stat_ofp) and os.path.exists(new_path_ofp):
                    print("{} exist. skip...".format(trial_name))
                    continue                
        
                print("run {}".format(trial_name))    

                # we only require 1-robust, so --kDelay 1
                cmd = f"ulimit -Sv {MAX_VIRTUAL_MEMORY} &&" \
                    f" {exe_path} -p {path_file_path} -s {sit_file_path}" \
                    f" -t {time_limit} -a {algo} -b {branch_order} -g {grouping_method} -h {heuristic} -e {early_termination} -i {incremental}" \
                    f" -o {stat_ofp} -n {new_path_ofp} --w_focal {w_focal}"
                
                cmds.append(cmd)
            
    # for cmd,output_name in zip(cmds,output_names):
    #     print(output_name,cmd)
            
    pool.starmap(run,zip(cmds,output_names))
    
subprocess.check_output(f"python script/get_stats.py -f {output_folder}", shell=True) 
subprocess.check_output(f"python script/get_stats.py -f {output_folder} -s", shell=True) 
subprocess.check_output(f"python script/get_stats.py -f {output_folder} -g", shell=True) 
subprocess.check_output(f"python script/get_stats.py -f {output_folder} -s -g", shell=True) 