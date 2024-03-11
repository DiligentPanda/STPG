OUTPUT_FOLDER="output/"
ALGO="graph"
BRANCH_ORDER="default"

mkdir -p $OUTPUT_FOLDER

./build/simulate -p example/path/map_lak303d_ins_1_an_31.path -s example/sit/map_lak303d_ins_1_an_31_sit_0.json \
 -t 90 -a $ALGO -b ${BRANCH_ORDER} -o ${OUTPUT_FOLDER}${ALGO}_stats.txt -n ${OUTPUT_FOLDER}${ALGO}_new_paths.txt