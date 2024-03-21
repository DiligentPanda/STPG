fail_output_folder="example/fail"
stat_output_folder="example/stat"
path_output_folder="example/path"
time_limit=180

import os

for stat_output_file_name in os.listdir(stat_output_folder):
    stat_output_file_path=os.path.join(stat_output_folder,stat_output_file_name)
    with open(stat_output_file_path) as f:
        line=f.readline()
        if line.find(",-1,")!=-1:
            os.remove(stat_output_file_path)
            path_output_file_name=stat_output_file_name.replace(".stat",".path")
            path_output_file_path=os.path.join(path_output_folder,path_output_file_name)
            try:
                os.remove(path_output_file_path)
            except:
                pass
            fail_output_file_name=stat_output_file_name.replace(".stat",".fail")
            fail_output_file_path=os.path.join(fail_output_folder,fail_output_file_name)
            with open(fail_output_file_path,'w') as f:
                f.write("No solution found. ")
                records=line.strip().split(",")
                t=float(records[0])
                if t>time_limit:
                    f.write("Timeout: {}".format(time_limit))
                else:
                    f.write("No solution?")
            
            