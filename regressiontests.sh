#!/bin/sh

echo "testing list statistics calculations..."
g++ -O3 -std=c++11 listStats.cpp regtest.cpp -o rtests
./rtests > testdata/test_stats.txt
diff testdata/test_stats.txt testdata/correct_stats.txt

echo "testing selector table generation..."
rm test_sg
rm testdata/test_sg.txt
g++ -g -std=c++11 listStats.cpp selectorGen.cpp sg_test.cpp -o test_sg
./test_sg testdata/sg_testdata.bin > testdata/test_sg.txt
diff testdata/test_sg.txt testdata/correct_selector_gen.txt

valgrind ./test_sg > /dev/null testdata/sg_testdata.bin
