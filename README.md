# Speedup Techniques for Switchable Temporal Plan Graph 
This repo contains code for the paper Speedup Techniques for Switchable Temporal Plan Graph (He Jiang, Muhan Lin, Jiaoyang Li, AAAI 2025).

1. This repo contains two submodules `PBS` and `pybind11`. Please use `git clone --recursive https://github.com/DiligentPanda/STPG.git` to download the code. If you already download code with only `git clone`, you can use `git submodule init` and `git submodule update` to download submodules.
2. This repo contains both c++ and python code. To compile c++ code, you need to install cmake.
3. To compile the code, you can run `compile.sh`.
4. To run experiments, you can run `run_exps.sh`. All the data are located in the `data` folder.
5. `analysis` folder contains scripts to generate figures.
