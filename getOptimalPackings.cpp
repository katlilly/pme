#include <stdio.h>
#include <cstdlib>
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
	//int **selectors = new int* [NUMDOCS];
		
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
	
		if (listnumber == 95)
			{
			int *columns = new int [32];
			int bits_used = 0;
			uint column = 0;
			uint current = 0;
			int column_width = 0;
			int *frequencies = new int [32];
			memset(frequencies, 0, 32*4);

			int num_512bit_words = 0;
			//int num_unique_selectors = 0;

			int *bits_in_huff_code = new int [32];

			// huffman code for list #95
			bits_in_huff_code[2] = 5;
			bits_in_huff_code[3] = 4;
			bits_in_huff_code[4] = 1;
			bits_in_huff_code[5] = 2;
			bits_in_huff_code[6] = 3;
			bits_in_huff_code[7] = 5; 
			
			int maxselectorbits = 0;
			
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
					//selectors[num_512bit_words] = new int[column + 1];//= columns;
					//selectors[num_512bit_words][0] = column;
					//for (uint i = 1; i < column + 1; i++)
					//	selectors[num_512bit_words][i] = columns[i];
							  
					}

				num_512bit_words++;
				int sum = 0;
				uint i;
				int bitsneeded = 0;
				for (i = 0; i < column; i++)
					{
					sum += columns[i];
					bitsneeded += bits_in_huff_code[columns[i]];
					//printf("%d ", columns[i]);
					}
				if (bitsneeded > maxselectorbits)
					maxselectorbits = bitsneeded;
				//printf("\nselector can be encoded in %d bits\n", bitsneeded);
				//printf("     %d ints packed into %d bits\n", i, sum);
				}
			//printf("%d: packed %d words using %d unique selectors\n", length, num_512bit_words, num_unique_selectors);
			printf("list %d, length %d, bits per dgap %.2f\n", listnumber, length, (double) 512 * num_512bit_words / length);
			printf("max selector bits: %d\n", maxselectorbits);
			int unique_widths = 0;
			for (int i = 0; i < 32; i++)
				{
				//printf("%d: %d\n", i, frequencies[i]);
				if (frequencies[i])
					unique_widths++;
				}
				
			printf("number of unique widths in this list %d\n", unique_widths);
			//	printf("%d: %d\n", i, frequencies[i]);


			printf("list length: %d, AVX words: %d\n", length, num_512bit_words);
			printf("dgaps per 512-bit word: %.1f\n", (double) length / num_512bit_words);

			delete [] bits_in_huff_code;
			delete [] columns;
			delete [] frequencies;

		
		}

		listnumber++;
		}

	
	delete [] dgaps;
	delete [] postings_list;
	
	return 0;
	}
