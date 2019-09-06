#!/bin/sh

g++ -O3 -std=c++11 listStats.cpp regtest.cpp -o rtests
./rtests > test_stats.txt
diff test_stats.txt correct_stats.txt

rm test_pme
rm test_sg.txt
g++ -std=c++11 listStats.cpp selectorGen.cpp pme_test.cpp -o test_pme
./test_pme postings.bin > test_sg.txt
diff test_sg.txt correct_selector_gen.txt

