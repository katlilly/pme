#!/bin/sh

rm test_pme
g++ -O3 -std=c++11 listStats.cpp selectorGen.cpp pme_test.cpp -o test_pme
./test_pme postings.bin

