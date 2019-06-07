#!/bin/sh

g++ -std=c++11 -fprofile-arcs -ftest-coverage listStats.cpp regtest.cpp -o testcov
./testcov testdata.bin
gcov listStats.cpp regtest.cpp
