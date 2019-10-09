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

// return the number of bytes that would be used in run length encoding
int get_rle_size(int *source, int length)
	{
	int count = 0;
	int runlength = 0;
	for (int index = 0; index < length; )
		{
		int current = source[index];
		if (current == source[index + 1])
			{
			// keep looking for end of run
			runlength++;
			index++;
			}
		else
			{
			// have found the end of a run, now write it out
			index++;
			while(runlength > 7)
				{
				//dest[count] = 0;
				//dest[count] |= 7;
				//dest[count] |= current << 3;
				runlength -= 8;
				count++;
				}
			//dest[count] = 0;
			//dest[count] |= runlength;
			//dest[count] |= current << 3;
			count++;
			current = source[index];
			runlength = 0;
			}
		}
	return count; // return number of bytes written
	}

struct record {
	int dgaps_compressed;
	int payload_bytes;
	int num_selectors;
	};


void print_record(record r, int name, int length)
	{
	printf("list number: %d, length: %d\n", name, length);
	printf("payload = %d bytes\n", r.payload_bytes);
	printf("total columns = %d\n\n", r.num_selectors);
	}


record compress(int *payload, int *selectors, int *raw, int length)
	{
	record result;
	result.dgaps_compressed = length;
	result.payload_bytes = 0;
	int *columns = new int[10000];
	int column, bits_used, column_width;
	int current = 0;
	int total_columns = 0;

	while (current < length)
		{
		// find column widths for a single word
		column_width = 0;
		column = 0;
		for (bits_used = 0; bits_used < 32; bits_used += column_width)
			{
			for (int row = 0; row < 16; row++)
				if (current + row < length)
					column_width |= raw[current + row];

			if (current > length)
				break;

			column_width = get_bitwidth(column_width);
			if (column_width + bits_used <= 32)
				{
				columns[column++] = column_width;
				current += 16;
				}
			}
		result.payload_bytes += 64;

		/*
		  append columns in this word to array of raw selectors
		*/
		for (int i = 0; i < column; i++)
			selectors[total_columns + i] = columns[i];

		total_columns += column;
		}

	delete [] columns;
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
	int *payloads = new int [NUMDOCS];
	int *selectors = new int [10000];

	int *compressed_selectors = new int [10000];
	int *selector_selectors = new int [10000];

	int *cc_selectors = new int [10000];
	int *ss_selectors = new int [10000];

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

		/*
		   Compress dgaps
		 */
		record result = compress(payloads, selectors, dgaps, length);
//		for (int i = 0; i < result.num_selectors; i++)
//			printf("%d, ", selectors[i]);
//		printf("\n");
//		print_record(result, listnumber, length);

		if ((int) length < (result.num_selectors - 1) * 16)
			exit(printf("too many columns\n"));
		if ((int) length > (result.num_selectors - 1) * 16 + 16) 
			exit(printf("too few columns\n"));

		if (result.num_selectors > 1000)
			{
			/*
			  Encode the selectors as if they were dgaps
			 */
//			printf("=======================\n");
//			print_record(result, listnumber, length);
			record secondaryresult = compress(compressed_selectors, selector_selectors, selectors, result.num_selectors);
//			print_record(secondaryresult, listnumber, result.num_selectors);

			// one more time
			record tertiaryresult = compress(cc_selectors, ss_selectors, selector_selectors, secondaryresult.num_selectors);
			

			
			/*
			  Run length encode the selectors of the selectors
			 */
			int initial_size = result.payload_bytes + get_rle_size(selectors, result.num_selectors);
			int depth1_size = result.payload_bytes + secondaryresult.payload_bytes +
				get_rle_size(selector_selectors, secondaryresult.num_selectors);
			int depth2_size = result.payload_bytes + secondaryresult.payload_bytes +
				tertiaryresult.payload_bytes + get_rle_size(ss_selectors, tertiaryresult.num_selectors);
//printf("non recursive size: %d bytes\n      depth 1 size: %d bytes\n      depth 2 size: %d bytes\n", initial_size, depth1_size, depth2_size);
			printf("%d, %d, %d, %d, %d\n", listnumber, length, initial_size, depth1_size, depth2_size);
			}
		
		listnumber++;
		}
	
	delete [] compressed_selectors;
	delete [] selector_selectors ;
	delete [] payloads;
	delete [] selectors;
	delete [] dgaps;
	delete [] postings_list;
	fclose(fp);
		
	return 0;
	}
