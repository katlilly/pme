#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include "../listStats.h"
#include "../selectorGen.h"

#define NUMDOCS (1024 * 1024 * 128)
#define NUMLISTS 499692

int main(int argc, char *argv[])
	{
	const char *filename;
	if (argc == 2)
		filename = argv[1];
	else
		exit(printf("Usage: %s <binfile>\n", argv[0]));
	FILE *fp;
	if (NULL == (fp = fopen(filename, "rb")))
		exit(printf("Cannot open %s\n", filename));

	uint32_t length;
	int listnumber = 0;
	int *postings_list = new int[NUMDOCS];
	int *bitwidths = new int[NUMDOCS];

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
		int selectorsize = 4;
		
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

			generator.sort_table(*table);
			generator.print_stats();
			generator.print_table(*table);


			for (int i = 0; i < table_size; i++)
				delete [] table->rows[i].bitwidths;
			delete [] table->rows;
			delete table;
			}

		listnumber++;
		}

	delete [] postings_list;
	delete [] bitwidths;

	fclose(fp);
  
	return 0;
	}
