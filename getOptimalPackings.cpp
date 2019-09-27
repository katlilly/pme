#include <stdio.h>
#include <cstdlib>

#define NUMDOCS (1024 * 1024 * 128)
#define NUMLISTS 499692

/* 
	efficiently calculate the number of bits needed for the binary
	representation. Copied from here:
	https://github.com/torvalds/linux/blob/master/include/asm-generic/bitops/fls.h
 */
int getBitwidth(uint x)
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
	
	while (fread(&length, sizeof(length), 1, fp) == 1)
		{
		if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
			exit(printf("error reading in postings list, listnumber: %d\n", listnumber));

		dgaps[0] = postings_list[0];
		int prev = postings_list[0];
		for (int i = 1; i < length; i++)
			{
			dgaps[i] = postings_list[i] - prev;
			prev = postings_list[i];
			}

		// check that above is correct by going back to original and comparing
		// to do
		
				
		if (listnumber == 95)
			{
		
			/*
			  Accumulator array for counting unique bitwidths in current postings list;
			*/
			int *packings = new int [32];
			int bitsused = 0;
			int column = 0;
			int current = 0;
			int result;
			while (current < length)
				{
				printf("new word\n");
				// find the optimal packing for next 512bits
				while (bitsused < 32 && current < length)
					{
					printf("new column\n");
					//printf
					// find bitwidth required for current column
					result = 0;
					for (int row = 0; row < 16; row++)
						if (current + row < length)
							result |= dgaps[current + row];
					result = getBitwidth(result);
					printf("column width = %d\n", result);
			
					bitsused -= result;
					current += 16;
					column++;
					}
				}
			delete [] packings;
		}


		if (length > 100)
			{
			printf("listnumber %d, length: %d\n", listnumber, length);

			int pos = 0;
			while (pos < length)
				{
				printf("%d, ", dgaps[pos++]);
				}

			}

		listnumber++;
		}
	delete [] dgaps;
	delete [] postings_list;
	
	return 0;
	}
