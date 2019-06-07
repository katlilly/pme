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
    exit(printf("Usage::%s <binfile>\n", argv[0]));
  FILE *fp;
  if (NULL == (fp = fopen(filename, "rb")))
    exit(printf("Cannot open %s\n", filename));

  int *postings_list = new int[NUMDOCS];
  int *dgaps = new int[NUMDOCS];
  int *bitwidths = new int[NUMDOCS];
  uint32_t length, listnumber = 0;

  /*
    Read in postings list - each list begins with its own length
   */
  while (fread(&length, sizeof(length), 1, fp) == 1)
  {
    if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
      exit(printf("something went wrong\n"));
    listnumber++;

    //printf("list number: %d\n", listnumber);
    ListStats ls(listnumber, length);
    ls.docnums_to_dgaps(dgaps, postings_list, length);
    ls.docnums_to_dgap_bitwidths(bitwidths, postings_list, length);
    if (listnumber)
      {
	ls.calculate_stats(bitwidths, length);
	
      }
    
    if (listnumber == 766)// || listnumber == 64)
    {
      //for (int i = 0; i < length; i++)
      //printf("%d, ", bitwidths[i]);

    }
    
  }

  


  
  return 0;
}
