#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <string.h>

#define NUMDOCS (1024 * 1024 * 128)
#define NUMLISTS 499692

/* 
	efficiently calculate the number of bits needed for the binary
	representation. Copied from here:
	https://github.com/torvalds/linux/blob/master/include/asm-generic/bitops/fls.h
*/
int get_bitwidth(uint x)
	{
	int r = 32;

	if (!x)
		return 1;
	if (!(x & 0xffff0000u))
		{
		x <<= 16;
		r -= 16;
		}
	if (!(x & 0xff000000u))
		{
		x <<= 8;
		r -= 8;
		}
	if (!(x & 0xf0000000u))
		{
		x <<= 4;
		r -= 4;
		}
	if (!(x & 0xc0000000u))
		{
		x <<= 2;
		r -= 2;
		}
	if (!(x & 0x80000000u))
		{
		x <<= 1;
		r -= 1;
		}
	return r;
	}


typedef struct record {
	int dgaps_compressed;
	int num_512bit_words;
	int num_selectors;
	};


record compress(int *payload, int *selectors, int *raw, int length)
	{
	record result;
	result.dgaps_compressed = length;

	int column, bits_used, column_width;
	int current = 0;

	while (current < length)
		{
		// find column widths for a single word
		column = 0;
		for (bits_used = 0; bits_used < 32; bits_used += column_width)
			{
			for (uint row = 0; row < 16; row++)
				if (current + row < length)
					column_width |= dgaps[current + row];

			if (current > length)
				break;

			column_width = get_bitwidth(column_width);
			if (column_width + bits_used <= 32)
				{
				columns[column++] = column_width;
				current += 16;
				}
			}
		num_512bit_words++;

		/*
		  append columns in this word to array of raw selectors
		*/
		for (uint i = 0; i < column; i++)
			raw_selectors[total_columns + i] = columns[i];

		total_columns += column;
		}

	result.num_512bit_words = num_512bit_words;
	result.num_selectors = total_columns;
	return result;
	}


int main(int argc, char *argv[])
	{
	const char *filename = "postings.bin";
	FILE *fp;
	if (NULL == (fp = fopen(filename, "rb")))
		exit(printf("Cannot open %s\n", filename));

	int listnumber = 0;
	uint length;
	int *postings_list = new int[NUMDOCS];
	int *dgaps = new int[NUMDOCS];

	int *raw_selectors = new int [10000]; // in wsj postings.bin, the largest number of columns per list is 9867
	int *selector_selectors = new int [10000];
	
	while (fread(&length, sizeof(length), 1, fp) == 1)
		{
		if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
			exit(printf("error reading in postings list, listnumber: %d\n", listnumber));

		/* 
			Convert docnums to dgaps
		*/
		dgaps[0] = postings_list[0];
		int prev = postings_list[0];
		for (uint i = 1; i < length; i++)
			{
			dgaps[i] = postings_list[i] - prev;
			prev = postings_list[i];
			}

		int *columns = new int [32];
		int bits_used = 0;
		uint column = 0;
		uint current = 0;
		int column_width = 0;
		int num_512bit_words = 0;
		int total_columns = 0;

		/*
		  find selectors
		*/
		
		
		while (current < length)
			{
			/*
			  find column widths for a single word
			*/
			column = 0;
			for (bits_used = 0; bits_used < 32; bits_used += column_width)
				{
				for (uint row = 0; row < 16; row++)
					if (current + row < length)
						column_width |= dgaps[current + row];

				if (current > length)
					break;

				column_width = get_bitwidth(column_width);
				if (column_width + bits_used <= 32)
					{
					columns[column++] = column_width;
					current += 16;
					}
				}
			num_512bit_words++;

			/*
			  append columns in this word to array of raw selectors
			*/
			for (uint i = 0; i < column; i++)
				raw_selectors[total_columns + i] = columns[i];

			total_columns += column;
			}


		/*
		  find selectors for selectors
		*/

		int num_512bit_words_for_selectors = 0;
		int selector_columns = 0;
		while (current < total_columns)
			{
			/*
			  find column widths for a single word
			*/
			column = 0;
			for (bits_used = 0; bits_used < 32; bits_used += column_width)
				{
				for (uint row = 0; row < 16; row++)
					if (current + row < total_columns)
						column_width |= raw_selectors[current + row];

				if (current > total_columns)
					break;

				column_width = get_bitwidth(column_width);
				if (column_width + bits_used <= 32)
					{
					columns[column++] = column_width;
					current += 16;
					}
				}
			num_512bit_words_for_selectors++;

			/*
			  append columns in this word to array of raw selectors
			*/
			
			for (uint i = 0; i < column; i++)
				selector_selectors[total_columns + i] = columns[i];

			selector_columns += column;
			}

		printf("created %d selectors, and packed those with %d selectors\n", total_columns, selector_columns);
			
			
		delete [] columns;
			
		//printf("%d\n", num_512bit_words);
		listnumber++;
		}

	delete [] dgaps;
	delete [] postings_list;

	return 0;
	}
