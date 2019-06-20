#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include "listStats.h"
#include "fls.h"

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
  int *bitwidths = new int[NUMDOCS];
  uint32_t length, listnumber = 0;
  double sum = 0;
  int count = 0;

  printf("length, ints/word, highFrac\n");
  
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


    if (ls.mode + ls.mode + ls.highexcp <= 28)
      {
	int intstopack = 28 / ls.mode;
	printf("%5u %d %.2f\n", length, intstopack, ls.highFrac);
	sum += ls.highFrac;
	count++;
      }
  }
  double mean = sum / count;
  printf("unweighted mean highexp frequency: %.2f\n", mean);
  

  delete [] postings_list;
  delete [] bitwidths;

  fclose(fp);
  
  return 0;
}
