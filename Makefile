
all : recursive_compress get_selectors get_widths get_unique test_stats test_sg test_sel_size test_compression get_optimal_packing_data

recursive_compress : recursiveCompression.cpp
	g++ -g -Wall -std=c++11 recursiveCompression.cpp -o recursive_compress

get_selectors : getSelectors.cpp runLengthEncode.cpp
	g++ -Wall -std=c++11 runLengthEncode.cpp getSelectors.cpp -o get_selectors

get_widths : get_columns_and_widths.cpp
	g++ -Wall -std=c++11 get_columns_and_widths.cpp -o get_widths

get_unique : get_num_unique_selectors.cpp
	g++ -Wall -std=c++11 get_num_unique_selectors.cpp -o get_unique

get_optimal_packing_data : getOptimalPackings.cpp
	g++ -g -Wall -std=c++11 getOptimalPackings.cpp -o get_optimal_packing_data

test_compression : listStats.cpp compressPME.cpp selectorGen.cpp tests/compression_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp compressPME.cpp selectorGen.cpp tests/compression_test.cpp -o test_compression

test_sel_size : listStats.cpp selectorGen.cpp tests/selectorsize_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp tests/selectorsize_test.cpp -o test_sel_size

test_stats : listStats.cpp tests/stats_test.cpp
	g++ -O3 -Wall -std=c++11 listStats.cpp tests/stats_test.cpp -o test_stats

test_sg : listStats.cpp selectorGen.cpp tests/sg_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp tests/sg_test.cpp -o test_sg

clean:
	- rm -f *.gcno *.gcda *.gcov recursive_compress get_unique get_columns get_selectors get_widths get_optimal_packing_data test_compression test_pme test_sort test_stats test_sg test_sel_size testdata/test_sg.txt testdata/test_sg.txt *# *~ tests/*~
