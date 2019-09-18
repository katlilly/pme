#!/bin/sh

make clean

g++ -g -O3 -Wall -std=c++11 listStats.cpp stats_test.cpp -o test_stats
valgrind --track-origins=yes --leak-check=full ./test_stats > /dev/null

g++ -g -O3 -Wall -std=c++11 listStats.cpp selectorGen.cpp sg_test.cpp -o test_sg
valgrind --track-origins=yes --leak-check=full ./test_sg testdata/sg_testdata.bin > /dev/null
