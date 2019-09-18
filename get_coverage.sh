#!/bin/sh

make clean

g++ -std=c++11 -fprofile-arcs -ftest-coverage listStats.cpp stats_test.cpp -o test_stats
./test_stats > /dev/null

gcov listStats.cpp stats_test.cpp

g++ -std=c++11 -fprofile-arcs -ftest-coverage listStats.cpp selectorGen.cpp sg_test.cpp -o test_sg
./test_sg testdata/sg_testdata.bin > /dev/null


gcov listStats.cpp stats_test.cpp selectorGen.cpp

