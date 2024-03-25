set -ex

./compile.sh

OUTPUT_FOLDER="output/"
ALGO="graph"
BRANCH_ORDER="largest_diff"
USE_GROUPING=true
HEURISTIC="wcg_greedy"

mkdir -p $OUTPUT_FOLDER

# ./build/simulate -p test/test_3.path -s test/test_3_sit_1.json \
#  -t 90 -a $ALGO -b ${BRANCH_ORDER} --use_grouping ${USE_GROUPING} -o ${OUTPUT_FOLDER}${ALGO}_stats.txt -n ${OUTPUT_FOLDER}${ALGO}_new_paths.txt

 ./build/simulate -p example/path/map_Paris_1_256_ins_19_an_120.path -s example/sit/map_Paris_1_256_ins_19_an_120_sit_3.json \
 -t 90 -a $ALGO -b ${BRANCH_ORDER} -g ${USE_GROUPING} -h ${HEURISTIC} -o ${OUTPUT_FOLDER}${ALGO}_stats2.txt -n ${OUTPUT_FOLDER}${ALGO}_new_paths.txt