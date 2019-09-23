#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <time.h>
#include "../listStats.h"
#include "../selectorGen.h"

#define NUMDOCS (1024 * 1024 * 128)
#define NUMLISTS 499692

int compare_rows(const void *a, const void *b)
	{
	const SelectorGen::selector_row *ra = (const SelectorGen::selector_row *) a;
	const SelectorGen::selector_row *rb = (const SelectorGen::selector_row *) b;
	int la = ra->length;
	int lb = rb->length;
	return la < lb ? 1 : la == lb ? 0 : -1;
	}


int main(int argc, char *argv[])
	{
	const char *filename;
	if (argc == 2)
		filename = argv[1];
	else
		exit(printf("Usage: %s <binfile>\n", argv[0]));
	FILE *fp;

	uint32_t length;
	int listnumber = 0;
	int *postings_list = new int[NUMDOCS];
	int *bitwidths = new int[NUMDOCS];

	clock_t start, end;
	double used;

	for (int selectorsize = 4; selectorsize < 9; selectorsize++)
		{
		if (NULL == (fp = fopen(filename, "rb")))
			exit(printf("Cannot open %s\n", filename));

		start = clock();
				
		/* 
			Read in each postings list (WSJ postings.bin) 
		*/
		while (fread(&length, sizeof(length), 1, fp)  == 1)
			{    
			/* 
				Read one postings list (and make sure we did so successfully) 
			*/
			if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
				exit(printf("error reading in postings list, listnumber: %d\n", listnumber));

			/*
			  Calculate bitwidth statistics for current list
			*/
			ListStats ls(listnumber, length);
			ls.docnums_to_dgap_bitwidths(bitwidths, postings_list, length);
			ls.calculate_stats(bitwidths, length);
			uint32_t encodedstats;
			ls.encode_stats(&encodedstats);
	
			/* 
				Generate a selector table for current list based on its statistics
			*/
			if (true)
				{
				SelectorGen generator(selectorsize, listnumber, encodedstats);
				SelectorGen::selector_table *table = new SelectorGen::selector_table;
				table->num_rows = 0;
				int table_size = generator.get_num_selectors();
				table->rows = new SelectorGen::selector_row[table_size];
				for (int i = 0; i < table_size; i++)
					table->rows[i].bitwidths = new int[generator.get_payload_bits()];

				generator.generate(table);
				qsort(table->rows, table->num_rows, sizeof(*table->rows), compare_rows);
				//generator.print_table(*table);
				
				for (int i = 0; i < table_size; i++)
					delete [] table->rows[i].bitwidths;
				delete [] table->rows;
				delete table;
				}

			listnumber++;
			}

		end = clock();
		used = ((double) (end - start)) / CLOCKS_PER_SEC;
		printf("selector size %d: %.2f seconds\n", selectorsize, used);

		fclose(fp);
		}
	
	delete [] postings_list;
	delete [] bitwidths;

	return 0;
	}
