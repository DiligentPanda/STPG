import subprocess

for map_name in ["city","game","random","warehouse"]:   
    ret=subprocess.check_output(f"python analysis/exp1_compare/plot_success_rates_{map_name}.py", shell=True)
    ret=subprocess.check_output(f"python analysis/exp1_compare/plot_execution_times_{map_name}.py", shell=True)
    print(ret)
