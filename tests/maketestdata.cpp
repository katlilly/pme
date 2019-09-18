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
  uint32_t length, listnumber = 0;
  FILE *output = fopen("testdata.bin", "w");
  FILE *output2 = fopen("sg_testdata.bin", "w");

  /*
    Read in postings list and write out the first 70 to a file to use
    for regression tests
   */
  while (fread(&length, sizeof(length), 1, fp) == 1)
  {
    if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
      exit(printf("something went wrong\n"));
    listnumber++;

    /* The first 70 lists contain some long lists, list 766 contains
       doc zero. this is to make testdataset for testing statistics
       calculation */
    if (listnumber < 71 || listnumber == 766)
    {
      fwrite(&length, sizeof(int), 1, output);
      fwrite(postings_list, sizeof(int), length, output);
    }

    /* 
       This is to create test data for testing selector table generation
     */
    if (listnumber == 64 || listnumber == 67 || listnumber == 84 ||
	listnumber == 96 || listnumber == 153 || listnumber == 143418 ||
	listnumber == 454789 || listnumber == 499691 || listnumber == 499692)
    {
      fwrite(&length, sizeof(int), 1, output2);
      fwrite(postings_list, sizeof(int), length, output2);
    }

  }

  delete [] postings_list;
  fclose(output);
  return 0;
}
