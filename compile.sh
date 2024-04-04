set -ex

mkdir -p build
cd build
cmake -DBOOST_ROOT=./boost_1_73_0 ..
make -j
cd ..