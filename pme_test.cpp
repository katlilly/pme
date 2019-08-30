#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include "fls.h"
#include "listStats.h"
#include "selectorGen.h"

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
    /* Read one postings list (and make sure we did so successfully) */
    if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
    {
      exit(printf("i/o error\n"));
    }
    //printf("list number %d, length = %d\n", listnumber, length);


    /*
      Calculate bitwidth statistics for current list
    */
    ListStats ls(listnumber, length);
    ls.docnums_to_dgap_bitwidths(bitwidths, postings_list, length);
    ls.calculate_stats(bitwidths, length);
    //ls.print_stats_short();

    /*
      Encode the calculated statistics into 32 bits, then decode into
      a struct
    */
    uint encodedstats;
    //printf("%d\n", encodedstats);
    ls.encode_stats(&encodedstats);
    ListStats::record stats = ls.decode_stats(&encodedstats);
    ls.print_stats_record(stats);


    /* 
       Generate a selector table for current list based on its
       decoded statistics
     */
    // obviously should change inputs so this takes the struct rather
    // than each of its elements
    int selectorbits = 4;
    SelectorGen generator(selectorbits, stats.lst, stats.md, stats.hxp, stats.hst);
    int numselectors = 16;
    int **table = new int*[numselectors];

    for (int i = 0; i < numselectors; i++)
      table[i] = new int[32 - selectorbits];
    generator.generate(table);
 
    generator.print_table(table);

    SelectorGen::selector_table st = generator.convert_table(table);
    //generator.print_converted_table(st);

    


    listnumber++;
  }

  delete [] postings_list;
  delete [] bitwidths;
  //for (int i = 0; i < numselectors; i++)
  //delete [] table[i];
  //delete [] table;

  fclose(fp);
  
  return 0;
}
  
