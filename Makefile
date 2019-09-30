
all : test_stats test_sg test_sel_size test_compression get_optimal_packing_data

get_optimal_packing_data : getOptimalPackings.cpp
	g++ -Wall -std=c++11 getOptimalPackings.cpp -o get_optimal_packing_data

test_compression : listStats.cpp compressPME.cpp selectorGen.cpp tests/compression_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp compressPME.cpp selectorGen.cpp tests/compression_test.cpp -o test_compression

test_sel_size : listStats.cpp selectorGen.cpp tests/selectorsize_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp tests/selectorsize_test.cpp -o test_sel_size

test_stats : listStats.cpp tests/stats_test.cpp
	g++ -O3 -Wall -std=c++11 listStats.cpp tests/stats_test.cpp -o test_stats

test_sg : listStats.cpp selectorGen.cpp tests/sg_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp tests/sg_test.cpp -o test_sg

clean:
	- rm -f *.gcno *.gcda *.gcov get_optimal_packing_data test_compression test_pme test_sort test_stats test_sg test_sel_size testdata/test_sg.txt testdata/test_sg.txt *# *~ tests/*~
