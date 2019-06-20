#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include "listStats.h"
#include "fls.h"
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

  int *postings_list = new int[NUMDOCS];
  int *dgaps = new int[NUMDOCS];
  int *bitwidths = new int[NUMDOCS];
  uint32_t length, listnumber = 0;
  int numselectors = 16;
  
  
  /*
    Read in postings list - each list begins with its own length
   */
  while (fread(&length, sizeof(length), 1, fp) == 1)
  {
    if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
      exit(printf("something went wrong, data doesn't match length\n"));
    listnumber++;

    /*
      Calculate statistics of current list
    */
    ListStats ls(listnumber, length);
    ls.docnums_to_dgap_bitwidths(bitwidths, postings_list, length);
    ls.calculate_stats(bitwidths, length);
    
    /*
      Encode list statistics into 32 bits, then decode into a struct
     */
    uint encodedstats;
    ls.encode_stats(&encodedstats);
    ListStats::record stats = ls.decode_stats(&encodedstats);
    
    if (ls.mode < 10)
      ls.print_stats_record(stats);

    SelectorGen generator(4, stats.lst, stats.hst, stats.hxp, stats.md);
    generator.table = new int*[numselectors];
    for (int i = 0; i < numselectors; i++)
      generator.table[i] = new int[28];
    generator.generate(generator.table);
    if (ls.mode < 10)
      generator.print_table(generator.table);

    
  }

  delete [] postings_list;
  delete [] dgaps;
  delete [] bitwidths;

  //for (int i = 0; i < numselectors; i++)
  //  delete [] generator.table[i];
  //delete [] generator.table;

  fclose(fp);
  
  return 0;
}
