#!/bin/sh

g++ -O3 -std=c++11 listStats.cpp regtest.cpp -o rtests
./rtests > test_stats.txt
diff test_stats.txt correct_stats.txt

rm test_sg
#rm test_sg.txt
g++ -g -std=c++11 listStats.cpp selectorGen.cpp sg_test.cpp -o test_sg
./test_sg sg_testdata.bin #> test_sg.txt
#diff test_sg.txt correct_selector_gen.txt

