all : test_stats test_sg

test_stats : listStats.cpp stats_test.cpp
	g++ -O3 -Wall -std=c++11 listStats.cpp stats_test.cpp -o test_stats

test_sg : listStats.cpp selectorGen.cpp sg_test.cpp
	g++ -g -Wall -std=c++11 listStats.cpp selectorGen.cpp sg_test.cpp -o test_sg

clean:
	- rm -f *.gcno *.gcda *.gcov test_pme test_stats test_sg testdata/test_sg.txt testdata/test_sg.txt



#pme: pme.c
#	$(CC) $(CFLAGS) -o pme pme.c -lm

#runpme: pme _build/Debug/postings.bin
#	./pme _build/Debug/postings.bin




#notsimple9: permutations.c notsimple9.c
#	$(CC) $(CFLAGS) -o notsimple9 notsimple9.c permutations.c -lm


#compare-notsimple9: notsimple9 postings.bin
#	./notsimple9 postings.bin


#compress: compress.c
#	$(CC) $(CFLAGS) -c compress.c


#wsj-compress: compress.c wsj-compress.c
#	$(CC) $(CFLAGS) -o wsj-compress wsj-compress.c compress.c


#do-wsj-compress: wsj-compress postings.bin
#	./wsj-compress postings.bin


