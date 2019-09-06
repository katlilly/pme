#!/bin/sh

g++ -g -std=c++11 listStats.cpp selectorGen.cpp sg_test.cpp -o test_sg
valgrind --track-origins=yes --leak-check=full ./test_sg sg_testdata.bin > test_sg.txt
