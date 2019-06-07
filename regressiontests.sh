#!/bin/sh

g++ -O3 -std=c++11 listStats.cpp regtest.cpp -o rtests
./rtests > test_stats.txt
diff test_stats.txt correct_stats.txt
