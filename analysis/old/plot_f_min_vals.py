stats_files={
    "largest_diff_no_group": "output/graph_stats_largest_diff_no_group.txt",
    "default_no_group": "output/graph_stats_default_no_group.txt",
    # "largest_diff_group": "output/graph_stats_largest_diff_group.txt",
    # "default_group": "output/graph_stats_default_group.txt",
    # "largest_diff_group": "output/graph_stats_no_inc_largest_diff.txt",
    # "default_group": "output/graph_stats_no_inc_default.txt"
}

output_file="analysis/temp/f_min_vals_no_group.pdf"

import matplotlib.pyplot as plt
import json
import numpy as np

for name, stats_file in stats_files.items():
    with open(stats_file, "r") as f:
        data=json.load(f)
        f_min_vals=data["open_list_f_min_vals"]
        plt.plot(np.arange(len(f_min_vals)),f_min_vals, label=name)

plt.legend()

plt.savefig(output_file, bbox_inches='tight', dpi=300)