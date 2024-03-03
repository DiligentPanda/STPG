import os
import time
import multiprocessing
import pandas as pd
import subprocess

exe_path="./build/sadg"
path_folder="example/path"
file_names_fp="example/path_file_names.csv"

timestamp=time.strftime("%Y_%m_%d_%H_%M_%S")
# TODO(rivers): merge output into one or separate them somehow?
output_folder="output/{}".format(timestamp)
delay_prob=10
delay_steps_low=10
delay_steps_high=20
time_limit=90
MAX_VIRTUAL_MEMORY = 8 * 1024 * 1024 # 8 GB
skip=False

instance_idxs=list(range(1,25+1)) # this is the number provided by the benchmark
num_trials=6

# setting: [agent_num_start, agent_num_end, agent_num_step, max_process_num]
maps = {
        "random-32-32-10":[25,50,5,32],
        "warehouse-10-20-10-2-1":[40,90,10,32],
        "Paris_1_256": [30,80,10,32],
        "lak303d": [15,35,4,32]
       }

main_output_folder=os.path.join(output_folder,"main")
fail_output_folder=os.path.join(output_folder,"fail")
aux_output_folder=os.path.join(output_folder,"aux")

os.makedirs(main_output_folder,exist_ok=True)
os.makedirs(fail_output_folder,exist_ok=True)
os.makedirs(aux_output_folder,exist_ok=True)

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
        for trial_idx in range(num_trials):
            trail_name="map_{}_ins_{}_an_{}_tr_{}".format(map_name,instance_idx,agent_num,trial_idx)
            output_names.append(trail_name)
            graph_stats_ofp=os.path.join(main_output_folder,trail_name+".graph")
            simul_stats_ofp=os.path.join(main_output_folder,trail_name+".simul")
            locations_ofp=os.path.join(aux_output_folder,trail_name+".locs")
            execution_ofp=os.path.join(aux_output_folder,trail_name+".exec")
            setup_ofp=os.path.join(aux_output_folder,trail_name+".setup")
            
            if skip and os.path.exists(graph_stats_ofp) and os.path.exists(simul_stats_ofp):
                print("{} exist. skip...".format(trail_name))
                continue                
    
            print("run {}".format(trail_name))    

            # we only require 1-robust, so --kDelay 1
            cmd = f"ulimit -Sv {MAX_VIRTUAL_MEMORY} && " \
                  f"{exe_path} -p {path_file_path} -d {delay_prob} -l {delay_steps_low} -h {delay_steps_high} -t {time_limit} " \
                  f"-g {graph_stats_ofp} -s {simul_stats_ofp} " \
                  f"-c {locations_ofp} -e {execution_ofp} -r {setup_ofp}"
            
            cmds.append(cmd)
    
    # for cmd,output_name in zip(cmds,output_names):
    #     print(output_name,cmd)
            
    pool.starmap(run,zip(cmds,output_names))
    

