set -ex
./compile.sh
./build/CBSH-rect-cmake/CBS-K --map_fp ../data/map/random-32-32-10.map --path_fp ../data/benchmark/test/map_random-32-32-10_ins_1_an_1.path --sit_fp ../data/benchmark/test/map_random-32-32-10_ins_1_an_1_sit_0.json \
-s CBSH-RM -t 90 --kDelay 1 --target True --corridor True --no-train-classify --ignore-train --printPath --stat_ofp test_ccbs_stats.txt 