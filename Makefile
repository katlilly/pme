
CC = gcc
CFLAGS = -ansi -pedantic -W -Wall -g


pme: pme.c
	$(CC) $(CFLAGS) -o pme pme.c

runpme: pme _build/Debug/postings.bin
	./pme _build/Debug/postings.bin

clean:
	rm -f pme



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


