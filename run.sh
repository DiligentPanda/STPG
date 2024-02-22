OUTPUT_FOLDER="output/"

mkdir -p $OUTPUT_FOLDER

./build/sadg example/path.txt 10 10 20 $OUTPUT_FOLDER/stats_graph.csv $OUTPUT_FOLDER/stats_exec.csv $OUTPUT_FOLDER/locations.txt $OUTPUT_FOLDER/delay_setup.txt