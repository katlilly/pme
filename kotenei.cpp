#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include "simple10avx.h"

#define NUMDOCS (1024 * 1024 * 128)
#define NUMLISTS 499692

// return type for list compression
struct record {
	int dgaps_compressed;
	int payload_bytes;
	int num_selectors;
	int selector_bytes;
	};

// return type for a single avx word within a list
struct thing {
	int n_compressed;
	int n_columns;
	};


void print_record(record r, int name, int length)
	{
	printf("list number: %d, length: %d\n", name, length);
	printf("payload = %d bytes\n", r.payload_bytes);
	printf("total columns = %d, %d bytes\n\n", r.num_selectors, r.selector_bytes);
	}


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

/* 
	Write list of selectors into selectors array and return the number
	of selectors generated
*/
int generate_selectors(int *selectors, int *dgaps, int *end)
	{
	int column = 0;
	int length = end - dgaps;
	int current = 0;

	while (current < length)
		{
		// get width of next column
		int column_width = 0;
		for (int row = 0; row < 16; row++)
			if (current + row < length)
				column_width |= dgaps[current + row];
				
		column_width = get_bitwidth(column_width);
		current += 16;

		// write out this column width to selector array
		selectors[column++] = column_width;
		}
	
	return column;
	}


 thing encode_one_word(int *payload, int *selectors, int num_selectors, int *raw, int *end)
	{
	thing th;
	int length = end - raw;

	__m512i compressedword = _mm512_setzero_epi32();
	__m512i indexvector = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	__m512i columnvector;

	int i = 0;
	for (int bitsused = 0; bitsused + selectors[i + 1] <= 32; bitsused += selectors[i])
		{
		// gather next 16 ints into a 512bit register
		columnvector = _mm512_i32gather_epi32(indexvector, raw, 4);

		// left shift input to correct "column"
		const uint8_t shift = bitsused; 
		columnvector = _mm512_slli_epi32(columnvector, shift);

		// pack this column of 16 dgaps into compressed 512bit word
		compressedword = _mm512_or_epi32(compressedword, columnvector);
		raw += 16;
		i++;
		}

	// write compressed data to memory as 32bit ints
	_mm512_i32scatter_epi32(payload, indexvector, compressedword, 4);

	th.n_compressed = 16 * i;
	th.n_columns = i;

	
//	/* 
//		Check if this is the last 512 bit word for this list. deal with
//		last word in list later. also this isn't even the correct check
//		for is it the last word
//	*/
//	if (length <= 16)
//		{
//		return th;
//		}

	/*
	  This is not the last 512 bits, no need to check for end of list for now
	*/
//	else
//		{
//		}
	
	return th;
	}



// for now this is only decoding first 512 bits
int decode(int *decoded, int *selectors, int num_selectors, int *payload)
	{
	int dgaps_decompressed = 0;

	
	/*
	  Load 512 bits of compressed data into a register
	*/
	__m512i indexvector = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	__m512i compressed_word = _mm512_i32gather_epi32(indexvector, payload, 4);

	/* 
		Declare a 512 bit vector for decoding as well as a bitmask vector
	*/
	__m512i decomp_vect;
	//__m512i mask_vector;

	/* 
		Decompress 512 bits of encoded data
	 */
	int i = 0;
	for (int bitsused = 0; bitsused + selectors[i + 1] <= 32; bitsused += selectors[i])
		{
		// get selector and create bitmask vector
		int width = selectors[0];
		int mask = pow(2, width) - 1;
		__m512i mask_vector = _mm512_set1_epi32(mask);
		
		// get 16 dgaps by ANDing mask with compressed word
		decomp_vect = _mm512_and_epi32(compressed_word, mask_vector);

		// write those 16 numbers to int array of decoded dgaps
		_mm512_i32scatter_epi32(decoded, indexvector, decomp_vect, 4);

		// right shift the remaining data in the compressed word
		compressed_word = _mm512_srli_epi32(compressed_word, width);

		dgaps_decompressed += 16;
		}
	
	
	return dgaps_decompressed;
	}


record avx_optimal_pack(int *payload, int *selectors, int num_selectors, int *raw, int *end)
	{
	record list;

	thing word = encode_one_word(payload, selectors, num_selectors, raw, end);

	list.dgaps_compressed = word.n_compressed;
	list.num_selectors = num_selectors;
	list.payload_bytes = 64;
	list.selector_bytes = 0;


	//	while (raw < end)
//		{
//	thing word = encode_one_word(payload, selectors, raw, end);
		
//		payload += 16;
//		selectors += word.n_columns;
//		raw += word.n_compressed;
	
//		list.dgaps_compressed += word.n_compressed;
//		list.num_selectors += word.n_columns;
//		list.payload_bytes += 64;
//		list.selector_bytes += word.n_columns; // for now
//		}
	return list;
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
	int *payload = new int [NUMDOCS];
	int *selectors = new int [10000];
	int *decoded = new int [NUMDOCS];
	
	//int *compressed_selectors = new int [10000];
	//int *selector_selectors = new int [10000];

	
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

		
		if (length > 512) // for now just encode one word, guaranteed to not be at end of list
			{
			// first create the selectors
			int num_selectors = generate_selectors(selectors, dgaps, dgaps + length);
						
			// then compress using list of selectors
			record result = avx_optimal_pack(payload, selectors, num_selectors, dgaps, dgaps + length);

			for (int i = 0; i < 16; i++)
				printf("%d: ", payload[i]);
			printf("\n\n");

			int ndecompressed = decode(decoded, selectors, num_selectors, payload);

			for (int i = 0; i < ndecompressed; i++)
				printf("%d = %d\n", dgaps[i], decoded[i]);
			printf("\n\n");
			
			exit(0);
			}
		
		listnumber++;
		}
	
	//delete [] compressed_selectors;
	//delete [] selector_selectors ;
	delete [] payload;
	delete [] selectors;
	delete [] dgaps;
	delete [] postings_list;
	fclose(fp);
		
	return 0;
	}
