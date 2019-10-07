#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <string.h>
#include "runLengthEncode.h"

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


int compare_selectors(int *selector_a, int length_a, int *selector_b, int length_b)
	{
	if (length_a != length_b)
		return 1;
	for (int i = 0; i < length_a; i++)
		if (selector_a[i] != selector_b[i])
			return 1;
	return 0;
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
	int *num_unique_perlist = new int [11];
	int total_columns;

	RunLengthEncode rl_encoder;
	int *raw_selectors = new int [10000]; // in wsj postings.bin, the largest number of columns per list is 9867
	uint8_t *compressed_selectors = new uint8_t [10000]; 
	
	while (fread(&length, sizeof(length), 1, fp) == 1)
		{
		if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
			exit(printf("error reading in postings list, listnumber: %d\n", listnumber));

		if (true)
			{
			printf("\n\nList number %d, length %d\n", listnumber, length);

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
			int *frequencies = new int [32];
			int num_512bit_words = 0;
			total_columns = 0;
			
			
			//uint8_t *compressed_selector = new uint8_t [10000];
			int compressed_selector_size = 0;

			while (current < length)
				{
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
						frequencies[column_width]++;
						current += 16;
						}
					}
				num_512bit_words++;

				/*
				  append collumn to array of raw selectors
				 */
				for (uint i = 0; i < column; i++)
					raw_selectors[total_columns + i] = columns[i];

				total_columns += column;
				}
			
			delete [] columns;
			delete [] frequencies;
			}

		/*
		  compress selectors with run length encoding
		 */
		for (int i = 0; i < total_columns; i++)
			printf("%d, ", raw_selectors[i]);
		printf("\n");
		
		int result = rl_encoder.runlength_encode(compressed_selectors, raw_selectors, total_columns);
				
		listnumber++;
		}

	delete [] dgaps;
	delete [] postings_list;
	delete [] num_unique_perlist;

	return 0;
	}
