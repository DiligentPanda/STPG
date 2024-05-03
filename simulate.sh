set -ex

./compile.sh

OUTPUT_FOLDER="output/"
ALGO="graph"
BRANCH_ORDER="largest_diff"
GROUPING_METHOD="simple"
HEURISTIC="wcg_greedy"
EARLY_TERMINATION=true
INCREMENTAL=true
W_FOCAL=1.0

mkdir -p $OUTPUT_FOLDER

# ./build/simulate -p test/test_3.path -s test/test_3_sit_1.json \
#  -t 90 -a $ALGO -b ${BRANCH_ORDER} --use_grouping ${USE_GROUPING} -o ${OUTPUT_FOLDER}${ALGO}_stats.txt -n ${OUTPUT_FOLDER}${ALGO}_new_paths.txt

 ./build/simulate -p data/benchmark/test_PBS2/path/map_random-32-32-10_ins_19_an_75.path -s data/benchmark/test_PBS2/sit/map_random-32-32-10_ins_19_an_75_sit_2.json \
 -t 90 -a $ALGO -b ${BRANCH_ORDER} -g ${GROUPING_METHOD} -h ${HEURISTIC} -e ${EARLY_TERMINATION} -o ${OUTPUT_FOLDER}${ALGO}_stats.txt -n ${OUTPUT_FOLDER}${ALGO}_new_paths.txt --w_focal ${W_FOCAL} -i ${INCREMENTAL}