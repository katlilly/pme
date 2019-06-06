#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include "listStats.h"
#include "fls.h"

#define NUMDOCS (1024 * 1024 * 128)
#define NUMLISTS 499692


int main(int argc, char *argv[])
{
  const char *filename = "testdata.bin";
  FILE *fp;
  if (NULL == (fp = fopen(filename, "rb")))
    exit(printf("Cannot open %s\n", filename));

  int *postings_list = new int[NUMDOCS];
  int *dgaps = new int[NUMDOCS];
  int *bitwidths = new int[NUMDOCS];
  int *bitwidths2 = new int[NUMDOCS];
  uint32_t length, listnumber = 0;
  
  /*
    Read in postings list - each list begins with its own length
  */
  while (fread(&length, sizeof(length), 1, fp) == 1)
  {
    if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
      exit(printf("something went wrong while reading in test input data\n"));

    listnumber++;

    /*
      Check one known input value
    */
    if (listnumber == 71)
      if (postings_list[0] != 0)
	exit(printf("this list was supposed to contain document zero\n"));
    
    /*
      Calculate dgaps and bitwidths
    */
    ListStats ls(listnumber, length);
    ls.docnums_to_dgaps(dgaps, postings_list, length);
    ls.dgaps_to_bitwidths(bitwidths2, dgaps, length);
    ls.docnums_to_dgap_bitwidths(bitwidths, postings_list, length);
    
    /*
      Check two different bitwidth calculations against each other
    */
    for (int i = 0; i < length; i++)
      if (bitwidths[i] != bitwidths2[i])
	exit(printf("list %u, bitwidth calculation error\n", listnumber));
    
    /*
      Calculate mean bitwidth and stddev for each list, which will be
      piped to a file to compare with known correct results
    */
    ls.calculate_stats(bitwidths, length);
    printf("length: %5u, mean bitwidth: %.2f +/- %.2f\n", length, ls.mean, ls.stdev);
    
    
   
    
  }

    
  return 0;
}
