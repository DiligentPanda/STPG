import subprocess
import os

for map_name in ["city","game","random","warehouse"]:   
    ret=subprocess.check_output(f"python analysis/exp1_compare/plot_success_rates_{map_name}.py", shell=True)
    ret=subprocess.check_output(f"python analysis/exp1_compare/plot_execution_times_{map_name}.py", shell=True)
    print(ret)

script_folder="analysis/exp2_ablations"
for script_name in os.listdir(script_folder):
    if script_name.endswith(".py"):
        ret=subprocess.check_output(f"python {os.path.join(script_folder,script_name)}", shell=True)
        print(ret)