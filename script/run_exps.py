import os
import time
import multiprocessing
import pandas as pd
import subprocess

exe_path="./build/simulate"
path_folder="example/path"
sit_folder="example/sit"
file_names_fp="example/path_file_names.csv"
exp_desc="exp" # describe the experiments

timestamp=time.strftime("%Y_%m_%d_%H_%M_%S")
exp_name="{}_{}".format(timestamp,exp_desc)
# TODO(rivers): merge output into one or separate them somehow?
output_folder="output/{}".format(exp_name)
path_list_ofp=os.path.join(output_folder,"path_file_names.csv")
delay_prob=10
delay_steps_low=10
delay_steps_high=20
time_limit=90
algos=["graph"] # ["graph","exec"]
branch_orders=["largest_diff","conflict","default"] #,"random","earliest"]
use_groupings=["false","true"]
heuristics=["zero","wcg_greedy"]
early_terminations=["true"]
MAX_VIRTUAL_MEMORY = 8 * 1024 * 1024 # 8 GB
skip=False

instance_idxs=list(range(1,25+1)) # this is the number provided by the benchmark
num_sits=6

subprocess.check_output("./compile.sh", shell=True) 

# setting: [agent_num_start, agent_num_end, agent_num_step, max_process_num]
maps = {
        # "random-32-32-10":[25,50,5,8],
        # "warehouse-10-20-10-2-1":[40,90,10,8],
        "Paris_1_256": [70,120,10,16],
        # "lak303d": [15,35,4,16]
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
    
    if agent_num<agent_num_start or agent_num>=agent_num_end+agent_num_step:
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
            for algo in algos:
                for branch_order in branch_orders: 
                    for use_grouping in use_groupings:
                        for heuristic in heuristics:
                            for early_termination in early_terminations:
                                trial_name="{}_algo_{}_br_{}_gp_{}_heu_{}_et_{}".format(sit_name,algo,branch_order,use_grouping,heuristic,early_termination)
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
                                    f" -t {time_limit} -a {algo} -b {branch_order} -g {use_grouping} -h {heuristic} -e {early_termination}" \
                                    f" -o {stat_ofp} -n {new_path_ofp} --weight_h 2.0"
                                
                                cmds.append(cmd)
            
    # for cmd,output_name in zip(cmds,output_names):
    #     print(output_name,cmd)
            
    pool.starmap(run,zip(cmds,output_names))
    

