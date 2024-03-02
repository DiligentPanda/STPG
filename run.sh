OUTPUT_FOLDER="output/"

mkdir -p $OUTPUT_FOLDER

./build/sadg -p example/path.txt -d 10 -l 10 -h 20 -t 90 \
 -g $OUTPUT_FOLDER/stats_graph.csv -s $OUTPUT_FOLDER/stats_exec.csv \
 -c $OUTPUT_FOLDER/locations.txt -r $OUTPUT_FOLDER/delay_setup.txt \
 -e $OUTPUT_FOLDER/execution.txt