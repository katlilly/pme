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
	//int *num_unique_perlist = new int [11];

	printf("List number, length, mean number of columns, number of unique column widths\n");
	
	while (fread(&length, sizeof(length), 1, fp) == 1)
		{
		if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
			exit(printf("error reading in postings list, listnumber: %d\n", listnumber));
		
		if (true)
			{
			printf("%d, %d, ", listnumber, length);
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
			int total_columns = 0;
			uint current = 0;
			int column_width = 0;
			int *frequencies = new int [32];
			memset(frequencies, 0, 32*4);
			int num_512bit_words = 0;
			
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
				//printf("%d columns in current word\n", column);
				total_columns += column;
				
				/* print selectors in such a way that i can count them with uniq */
				for (uint i = 0; i < column; i++)
					{
					//printf("%d ", columns[i]);
					}
				//printf("\n");
				}

			int num_unique = 0;
			for (int i = 0; i < 32; i++)
				{
				//printf("%d: %d\n", i, frequencies[i]);
				if (frequencies[i])
					num_unique++;
				}
			
			double mean_columns_per32 = (double) total_columns / num_512bit_words;
			printf("%.2f, ", mean_columns_per32);
			printf("%d\n", num_unique);
			
			delete [] columns;
			delete [] frequencies;
			} 

		listnumber++;
		}

	delete [] dgaps;
	delete [] postings_list;
//	delete [] num_unique_perlist;
	
	return 0;
	}
