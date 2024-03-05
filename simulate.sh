OUTPUT_FOLDER="output/"
ALGO="enhanced"

mkdir -p $OUTPUT_FOLDER

./build/simulate -p example/path/map_lak303d_ins_10_an_31.path -s example/sit/map_lak303d_ins_10_an_31_sit_3.json \
 -t 90 -a $ALGO -o ${OUTPUT_FOLDER}${ALGO}_stats.txt -n ${OUTPUT_FOLDER}${ALGO}_new_paths.txt