OUTPUT_FOLDER="output/"
ALGO="exec"

mkdir -p $OUTPUT_FOLDER

./build/simulate -p example/path/map_random-32-32-10_ins_1_an_25.path -s example/sit/map_random-32-32-10_ins_1_an_25_sit_1.json \
 -t 90 -a $ALGO -o ${OUTPUT_FOLDER}${ALGO}_stats.txt -n ${OUTPUT_FOLDER}${ALGO}_new_paths.txt