set -ex

mkdir -p build
cd build
cmake ..
make -j
cd ..

cd CBS_K
mkdir -p build
cd build
cmake ../CBSH-rect-cmake
make -j
cd ../..