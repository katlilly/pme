
all : test_stats test_sg test_sel_size test_sort test_compression

test_compression : listStats.cpp compressPME.cpp selectorGen.cpp tests/compression_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp compressPME.cpp selectorGen.cpp tests/compression_test.cpp -o test_compression

test_sort : listStats.cpp selectorGen.cpp tests/sort_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp tests/sort_test.cpp -o test_sort

run_test_sort :
	./test_sort testdata/testdata.bin

test_sel_size : listStats.cpp selectorGen.cpp tests/selectorsize_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp tests/selectorsize_test.cpp -o test_sel_size

test_stats : listStats.cpp tests/stats_test.cpp
	g++ -O3 -Wall -std=c++11 listStats.cpp tests/stats_test.cpp -o test_stats

test_sg : listStats.cpp selectorGen.cpp tests/sg_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp tests/sg_test.cpp -o test_sg

clean:
	- rm -f *.gcno *.gcda *.gcov test_compression test_pme test_sort test_stats test_sg test_sel_size testdata/test_sg.txt testdata/test_sg.txt *# *~ tests/*~
