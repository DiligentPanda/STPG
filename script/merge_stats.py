import os
import shutil
import pandas as pd
import subprocess

output_folder="output/0717_exp_comparison_p002"
input_folders = [
    "output/2024_07_17_18_38_01_paper_exp_comparison_p002",
    "output/2024_07_18_00_35_41_exp_comparison_p002_ccbs"
]

dfs=[]
for input_folder in input_folders:
    # copy results folder
    shutil.copytree(os.path.join(input_folder,"stat"), os.path.join(output_folder,"stat"),dirs_exist_ok=True)
    shutil.copytree(os.path.join(input_folder,"fail"), os.path.join(output_folder,"fail"),dirs_exist_ok=True)
    shutil.copytree(os.path.join(input_folder,"path"), os.path.join(output_folder,"path"),dirs_exist_ok=True)
    
    # read path_file_names.csv
    path_list_fp=os.path.join(input_folder,"path_file_names.csv")
    path_list=pd.read_csv(path_list_fp,index_col="index")
    dfs.append(path_list)
    
# merge path_file_names.csv
df=pd.concat(dfs, axis=0, ignore_index=True).drop_duplicates().reset_index()

df.to_csv(os.path.join(output_folder,"path_file_names.csv"),index_label="index")

subprocess.check_output(f"python script/get_stats.py -f {output_folder} -a", shell=True) 