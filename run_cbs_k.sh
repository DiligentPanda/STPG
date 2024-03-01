EXE="CBS_K/build/CBS-K"

$EXE -m example/map/random-32-32-10.map -a example/scen/scen-even/random-32-32-10-even-1.scen \
-o ./test -s CBSH-RM -t 90 --kDelay 1 --diff-k --screen 0 --corridor True --target True --shrink -k 50 --ignore-train --printPath 