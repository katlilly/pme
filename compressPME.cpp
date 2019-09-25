#include <stdio.h>
#include <cstdlib>
#include <math.h>
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
	uint which;                             // which row in selector table
	int column;                             // which element in bitwidth array
	int current;                            // count elements within this compressed word
	int topack;                             // min of ints per selector and remaining data to compress
	uint32_t *dgap = (uint32_t *) raw;      // pointer to current integer to be compressed
	uint32_t *start = raw;                  // pointer to what will be first dgap in current compressed word
	uint32_t *end = raw + n_to_compress;    // pointer to end of raw data
	
	/*
	  Chose the selector
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
				break;   // try next row if current integer doesn't fit
			column++;
			}
		if (dgap >= end)
			break;
		}
	
   /*
	  Pack data into 32 bits
	*/
	*dest = 0 | which;  // pack the selctor
	column = 0;
	int shift_distance = table->s_bits;
	for (current = 0; current < topack; current++)
		{
		*dest = *dest | raw[current] << shift_distance;
		shift_distance += table->rows[which].bitwidths[column++];
		}

	return topack;   // return number of dgaps compressed into current word
	}


CompressPME::record CompressPME::encode(uint32_t *dest, uint32_t *raw, SelectorGen::selector_table *table, int n_to_compress)
	{
	int total_compressed = 0;
	int compressed = 0;
	CompressPME::record result;
	result.compressed_size = 0;

	while (n_to_compress)
		{
		compressed = encode_one_word(dest++, raw + total_compressed, table, n_to_compress);
		total_compressed += compressed;
		n_to_compress -= compressed;
		result.compressed_size += 4;
		}
	
	result.n_dgaps_compressed = total_compressed;
	return result;
	}


uint CompressPME::decode_one_word(uint32_t *dest, uint32_t *compressed, SelectorGen::selector_table *table, uint n_to_decompress)
	{
	int bits;
	uint ints_out = 0;
	uint32_t mask = pow(2, table->s_bits) - 1;
	uint32_t payload = *compressed;
	int selector = payload & mask;
	payload = payload >> table->s_bits;
	
	for (int column = 0; column < table->rows[selector].length; column++)
		{
		if (ints_out < n_to_decompress)
			{
			bits = table->rows[selector].bitwidths[column];
			mask = pow(2, bits) - 1;
			dest[ints_out++] = payload & mask;
			payload = payload >> bits;
			}
		}
	
	return ints_out;
	}

/*
  Decode a list of PME-compressed d-gaps
*/
int CompressPME::decode(uint32_t *dest, uint32_t *compressed, SelectorGen::selector_table *table, uint n_to_decompress)
	{
	int n_decompressed = 0;
	int total_decompressed = 0;

	while (n_to_decompress > 0)
		{
		n_decompressed = decode_one_word(dest, compressed++, table, n_to_decompress);
		n_to_decompress -= n_decompressed;
		total_decompressed += n_decompressed;
		dest += n_decompressed;
		}

	return total_decompressed;
	}
