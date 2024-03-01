EXE="CBS_K/build/CBS-K"
TIME_LIMIT=90
AGENT_NUM=40
OUTPUT_PATH="test"
PATH_RESULT_PATH="path.txt"

$EXE -m example/map/random-32-32-10.map -a example/scen/scen-even/random-32-32-10-even-1.scen \
-o $OUTPUT_PATH -s CBSH-RM -t $TIME_LIMIT --kDelay 1 --screen 0 --corridor True --target True -k $AGENT_NUM --printPath --writePath $PATH_RESULT_PATH --no-train-classify --ignore-train