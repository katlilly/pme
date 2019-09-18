
all : test_stats test_sg

test_stats : listStats.cpp tests/stats_test.cpp
	g++ -O3 -Wall -std=c++11 listStats.cpp tests/stats_test.cpp -o test_stats

test_sg : listStats.cpp selectorGen.cpp tests/sg_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp tests/sg_test.cpp -o test_sg

clean:
	- rm -f *.gcno *.gcda *.gcov test_pme test_stats test_sg testdata/test_sg.txt testdata/test_sg.txt *# *~
