#include <stdio.h>
#include <cstdlib>
#include "compressPME.h"
#include "selectorGen.h"

int CompressPME::min(uint a, uint b)
	{
	return a < b ? a : b;
	}

int CompressPME::get_bitwidth(uint x)
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


int CompressPME::encode_one_word(uint32_t *dest, uint32_t *raw, SelectorGen::selector_table *table, int n_to_compress)
	{
	printf("using table with %d rows\n", table->num_rows);
	//for (uint row = 0; row < table->num_rows; row++)
	//	printf("row length: %d\n", table->rows[row].length);
	
	
	uint which;                               // which row in selector table
	int column;                              // which element in bitwidth array
	int current;                             // count elements within this compressed word
	int topack;                              // min of ints per selector and remaining data to compress
	uint32_t *dgap = (uint32_t *) raw;       // pointer to current integer to be compressed
	uint32_t *start = raw;                   // pointer to what will be first dgap in current compressed word
	uint32_t *end = raw + n_to_compress;     // pointer to end of raw data
	
	/*
	  Chose selector
	 */
	for (which = 0; which < table->num_rows; which++)
		{
		column = 0;
		dgap = start;
		topack = min(n_to_compress, table->rows[which].length);
		end = raw + topack;
		for (; dgap < end; dgap++)
			{
			if (get_bitwidth(*dgap) > table->rows[which].bitwidths[column])
				break;   // increment which row if current integer doesn't fit
			column++;
			}
		if (dgap >= end)
			break;
		}

	printf("chose selector %d\n", which);

	/*
	  Pack one word
	*/


	
	printf("return value (topack) = %d\n", topack);
	return topack;   // return number of dgaps compressed into current word
	}

int CompressPME::decode(uint32_t *dest, uint32_t *compressed, int n_dgaps_to_decompress)
	{

	return 0;
	}
