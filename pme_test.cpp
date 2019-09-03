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
    /* 
       Read one postings list (and make sure we did so successfully) 
    */
    if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
      exit(printf("i/o error\n"));

    /*
       Calculate bitwidth statistics for current list
    */
    ListStats ls(listnumber, length);
    ls.docnums_to_dgap_bitwidths(bitwidths, postings_list, length);
    ls.calculate_stats(bitwidths, length);
    uint encodedstats;
    ls.encode_stats(&encodedstats);
	
    /* 
       Generate a selector table for current list based on its decoded
       statistics
     */
    int selectorsize = 4;
    SelectorGen generator(selectorsize, encodedstats);
    SelectorGen::selector_table *newtable = new SelectorGen::selector_table;
    generator.generate(newtable);
    if (generator.high_exception ==1 )//|| generator.lowest == 1)
    {
      printf("list number: %d\n", listnumber);
      printf("list length: %d\n", length);

      generator.print_stats();
      generator.print_table(*newtable);
    }
      
    delete newtable;
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
  
