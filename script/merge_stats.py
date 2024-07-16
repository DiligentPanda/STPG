import os
import shutil
import pandas as pd

output_folder="output/0709_exp_comparison_p03_all"
input_folders = [
    "output/2024_06_05_12_49_41_exp_comparison_p03",
    "output/2024_07_07_23_34_27_exp_comparison_p03_ccbs_add_corridor"
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