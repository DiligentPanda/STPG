import os
import time
import multiprocessing
import pandas as pd
import subprocess

exe_path="./build/generate"
root_folder="data/benchmark/test_PBS2_sadg"
path_folder=os.path.join(root_folder,"path")
file_names_fp=os.path.join(root_folder,"path_file_names.csv")
output_folder=root_folder

delay_prob=10
delay_steps_low=10
delay_steps_high=20
time_limit=90
MAX_VIRTUAL_MEMORY = 1 * 1024 * 1024 # 1 GB
skip=False

instance_idxs=list(range(1,25+1)) # this is the number provided by the benchmark
num_sits=6

subprocess.check_output("./compile.sh", shell=True) 

# setting: [agent_num_start, agent_num_end, agent_num_step, max_process_num]
maps = {
        "sadg_warehouse": [40,70,10,8],
        # "random-32-32-10":[65,110,5,32],
        # "warehouse-10-20-10-2-1":[120,300,10,32],
        # "Paris_1_256": [70,300,10,32],
        # "lak303d": [49,169,4,32]
       }

sit_output_folder=os.path.join(output_folder,"sit")
fail_output_folder=os.path.join(output_folder,"sit_fail")

os.makedirs(sit_output_folder,exist_ok=True)
os.makedirs(fail_output_folder,exist_ok=True)

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
# df = df.reset_index(drop=True)

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
        
        output_names.append(path_file_name)   

        print("run {}".format(path_file_name))    

        # we only require 1-robust, so --kDelay 1
        cmd = f"ulimit -Sv {MAX_VIRTUAL_MEMORY} &&" \
            f" {exe_path} -p {path_file_path} -n {num_sits}" \
            f" -o {sit_output_folder} -d {delay_prob} -l {delay_steps_low} -h {delay_steps_high}"
        
        cmds.append(cmd)
        
    # for cmd,output_name in zip(cmds,output_names):
    #     print(output_name,cmd)
            
    pool.starmap(run,zip(cmds,output_names))
    

