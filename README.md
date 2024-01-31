# Multi-Agent-via-Switchable-ADG


The graph folder contains the graph interface functions and their implementation.

The ADG folder contains the interface and implementation of a TPG (which includes a graph and some auxiliary information such as the actual paths on a map).

The Algorithm folder contains the implementation of our heuristic search algorithm and a simulator class. The simulator class is used in the execution-based module of the algorithm.

The simulate.cpp file is used to simulate the delay (before replanning), print the start and goal locations for replanning, and drive the entire code.


The types.h file contains the type definitions for the data structures that we use.


(I usually run the following command to compile the code:

g++ -Wall -Werror -Ofast src/types.h src/graph/*.h src/graph/*.cpp src/ADG/*.h src/ADG/*.cpp src/Algorithm/*.h src/Algorithm/*.cpp src/simulate.cpp

And the following command to run the code:

./a.out {path input file} {delay probability -- an integer in [0, 1000]} {lower bound of the delay length -- an integer} {upper bound of the delay length -- an integer} {output file (stats) for the graph-based module} {output file (stats) for the execution-based module} {output file for the start and goal locations when a delay happens} {output file for the index of the delayed agents and the length of the delay}

I'm aware that the command is formatted in a very ugly way... I will clean it up soon. But for now an example of the command would be:

./a.out example/path.txt 10 10 20 stats_graph.csv stats_exec.csv locations.txt delay_setup.txt

This reads an example path, uses a delay probability = 1 percent and a delay length range = [10, 20], and creates two .csv files to record the output stats and two .txt files to record the information about the delay.

The stats columns are (from left to right):

runtime || runtime + the time for constructing the TPG || original total cost of the TPG || replanned total cost || original remaining cost of the TPG (the parts after the delay) || replanned remaining cost

The rest columns are about runtime breakdown and the number of search nodes -- which I'm assuming are not that interesting to you.
)
