#!/bin/sh

cd PlaneDetection/include/MRF2.2
make
cd ../..
mkdir build
cd build
cmake ..
make
cd ..
