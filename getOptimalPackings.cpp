#include <stdio.h>
#include <cstdlib>

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
	int length;
	int *postings_list = new int[NUMDOCS];
	int *dgaps = new int[NUMDOCS];
	int *check = new int [NUMDOCS];
	
	while (fread(&length, sizeof(length), 1, fp) == 1)
		{
		if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
			exit(printf("error reading in postings list, listnumber: %d\n", listnumber));

		/* 
			Convert docnums to dgaps
		 */
		dgaps[0] = postings_list[0];
		int prev = postings_list[0];
		for (int i = 1; i < length; i++)
			{
			dgaps[i] = postings_list[i] - prev;
			prev = postings_list[i];
			}
	
		if (length > 10000)
			{
			int *columns = new int [32];
			int bits_used = 0;
			int column = 0;
			int current = 0;
			int column_width;

			int num_512bit_words = 0;
			int num_unique_selectors = 0;
			
			while (current < length)
				{
				column = 0;
				for (bits_used = 0; bits_used < 32; bits_used += column_width)
					{
					for (int row = 0; row < 16; row++)
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
				int sum = 0;
				for (int i = 0; i < column; i++)
					{
					sum += columns[i];
					//printf("%d, ", columns[i]);
					}
				//printf("%      d bits used\n", sum);
				}
			//printf("%d: packed %d words using %d unique selectors\n", length, num_512bit_words, num_unique_selectors);
			//printf("mean bits per dgap: %.2f\n\n", (double) 512 * num_512bit_words / length);
			printf("%d, %.2f\n", length, (double) 512 * num_512bit_words / length);
			
			//while (current < length)
			//{
			//printf("new word\n");
			// find the optimal packing for next 512bits
			
			// while (bitsused < 32 && current < length)
			// 	{
			// 	printf("new column\n");
			// 	//printf
			// 	// find bitwidth required for current column
			// 	result = 0;
			// 	for (int row = 0; row < 16; row++)
			// 		if (current + row < length)
			// 			result |= dgaps[current + row];
			// 	result = getBitwidth(result);
			// 	printf("column width = %d\n", result);
			
			// 	bitsused -= result;
			// 	current += 16;
			// 	column++;
			// 	//printf("%d columns in this word\n", column);
			// 	}
				//}
			delete [] columns;
		}

		// if (length > 100)
		// 	{
		// 	printf("listnumber %d, length: %d\n", listnumber, length);

		// 	int pos = 0;
		// 	while (pos < length)
		// 		{
		// 		printf("%d, ", dgaps[pos++]);
		// 		}
		// 	}

		listnumber++;
		}
	
	delete [] dgaps;
	delete [] postings_list;
	
	return 0;
	}
