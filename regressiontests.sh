#!/bin/sh

g++ listStats.cpp regtest.cpp -o rtests
./rtests > mean_stdev.txt
#diff mean_stdev.txt correct_mean_stdev.txt


