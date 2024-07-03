set -ex
./compile.sh
./build/CBSH-rect-cmake/CBS-K -m ../data/map/random-32-32-10.map -a ../data/scen/scen-even/random-32-32-10-even-1.scen \
-o ./test -s CBSH-RM -t 90 -k 50 --kDelay 1 --target True --no-train-classify --ignore-train --printPath 