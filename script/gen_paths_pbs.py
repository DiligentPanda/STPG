import os, random, subprocess,time,glob
import resource
import multiprocessing
    
exe_path="PBS/build/pbs"
map_folder="data/map"
scen_folder="data/scen/scen-even"

# timestamp=time.strftime("%Y_%m_%d_%H_%M_%S")
root_output_folder="data/benchmark/test_PBS2_robust1_360s"
# if os.path.exists(root_output_folder):
#     print("output root folder {} exists. exit...".format(root_output_folder))
#     exit(1)
stat_output_folder=os.path.join(root_output_folder,"stat")
path_output_folder=os.path.join(root_output_folder,"path")
fail_output_folder=os.path.join(root_output_folder,"path_fail")

# algos = "CBSH-RM"
time_limit=360
MAX_VIRTUAL_MEMORY = 8 * 1024 * 1024 # 8 GB
skip=False

instance_idxs=list(range(1,25+1)) # this is the number provided by the benchmark

# setting: [agent_num_start, agent_num_end, agent_num_step, max_process_num]
maps = {
        "random-32-32-10":[60,60,5,32],
        "warehouse-10-20-10-2-1":[110,110,10,32],
        # "Paris_1_256": [250,300,10,32],
        "lak303d": [41,41,4,32]
       }

# Maximal virtual memory for subprocesses (in bytes).
# MAX_VIRTUAL_MEMORY = 3.5 * 1024 * 1024 * 1024 # 3 GB
# def limit_virtual_memory():
#     # The tuple below is of the form (soft limit, hard limit). Limit only
#     # the soft part so that the limit can be increased later (setting also
#     # the hard limit would prevent that).
#     # When the limit cannot be changed, setrlimit() raises ValueError.
#     resource.setrlimit(resource.RLIMIT_AS, (MAX_VIRTUAL_MEMORY, resource.RLIM_INFINITY))


os.makedirs(stat_output_folder,exist_ok=True)
os.makedirs(path_output_folder,exist_ok=True)
os.makedirs(fail_output_folder,exist_ok=True)

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
            stat_output_file_path = os.path.join(stat_output_folder,output_name+'.stat')
            with open(stat_output_file_path) as f:
                line=f.readline()
                line=f.readline()
                records=line.strip().split(",")
                cost=int(records[5])
            if cost>=0:
                print("[SUCC] {}".format(cmd))
                print(result.stdout)
            else:
                print("[FAIL] {}".format(cmd))
                print("Solution not found! Probably run out of time.")
                print(result.stdout)
                print(result.stderr)
                log_fail(output_name,result=result)
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
    
    cmds=[]
    output_names=[]
    map_file_name = "{}.map".format(map_name)
    map_file_path = os.path.join(map_folder,map_file_name)
    for instance_idx in instance_idxs:
        scen_file_name = "{}-even-{}.scen".format(map_name,instance_idx)
        scen_file_path = os.path.join(scen_folder,scen_file_name) 
        for agent_num in range(agent_num_start,agent_num_end+agent_num_step,agent_num_step):
            output_name='map_{}_ins_{}_an_{}'.format(map_name,instance_idx,agent_num)
            output_names.append(output_name)
            
            stat_output_file_path = os.path.join(stat_output_folder,output_name+'.stat')
            path_output_file_path = os.path.join(path_output_folder,output_name+'.path')
            
            if skip and os.path.exists(stat_output_file_path) and os.path.exists(path_output_file_path):
                print("{} exist. skip...".format(output_name))
                continue

            print("run {}".format(output_name))    

            # we only require 1-robust, so --kDelay 1
            cmd = f"ulimit -Sv {MAX_VIRTUAL_MEMORY} && " \
                  f"{exe_path} -m {map_file_path} -a {scen_file_path}" \
                  f" --sipp 0 -t {time_limit} -k {agent_num} -r 1" \
                  f" -o {stat_output_file_path} --outputPaths {path_output_file_path}"  
            
            cmds.append(cmd)
    
    # for cmd,output_name in zip(cmds,output_names):
    #     print(output_name,cmd)
            
    pool.starmap(run,zip(cmds,output_names))
    

