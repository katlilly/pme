#!/bin/sh

echo "testing list statistics calculations..."
make clean
make test_stats
./test_stats > testdata/test_stats.txt
diff testdata/test_stats.txt testdata/correct_stats.txt

echo "testing selector table generation..."
make clean
make test_sg
./test_sg testdata/sg_testdata.bin > testdata/test_sg.txt
diff testdata/test_sg.txt testdata/correct_selector_gen.txt
