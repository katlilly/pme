#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include "fls.h"
#include "listStats.h"
#include "selectorGen.h"
#include "compressPME.h"
#include "compressSimple9.h"

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
  uint32_t *dgaps = new uint32_t[NUMDOCS];
  uint32_t *decoded = new uint32_t[NUMDOCS];

  uint32_t length, listnumber = 0;
  int selectorbits = 4;
  int numselectors = 16;
  int **table = new int*[numselectors];
  for (int i = 0; i < numselectors; i++)
    table[i] = new int[32 - selectorbits];
  
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
    ls.docnums_to_u32dgaps(dgaps, postings_list, length);
    ls.docnums_to_dgap_bitwidths(bitwidths, postings_list, length);
    ls.calculate_stats(bitwidths, length);
    
    /*
      Encode list statistics into 32 bits, then decode into a struct
    */
    uint encodedstats;
    ls.encode_stats(&encodedstats);
    ListStats::record stats = ls.decode_stats(&encodedstats);

    /* 
      Generate selector table given selector length and list stats
    */
    SelectorGen generator(selectorbits, stats.lst, stats.hst, stats.hxp, stats.md);
    generator.generate(table);
        
    /*
      Do the pme compression
    */
    //CompressPME pme_compressor(selectorbits, table);
    //pme_compressor.print_selector_table();

    /*
      Do simple 9 compression for effectiveness comparision
    */
    CompressSimple9 *s9_compressor = new CompressSimple9();
    uint32_t numencoded = 0;
    uint32_t compressedwords = 0;
    uint32_t compressedints = 0;

    uint32_t *encoded = new uint32_t[length];
    for (compressedints = 0; compressedints < length; compressedints += numencoded)
      numencoded = s9_compressor->encode(encoded + compressedwords++,
					 dgaps + compressedints,
					length - compressedints);

    //printf("%u ints compressed into %u words\n\n\n", compressedints, compressedwords);

    // decompress
    //uint32_t *decoded = new uint32_t[length];
    // if (listnumber < 1000)
    // {
    //   int offset = 0;
    //   for (int i = 0; i < compressedwords; i++)
    // 	{
    // 	  offset += s9_compressor->decode(decoded, encoded[i], offset);
    // 	}
      
    //   // check for differences between original and decompressed
    // }

    
    delete [] encoded;
    free(s9_compressor);

  }

  delete [] postings_list;
  delete [] bitwidths;
  delete [] dgaps;
  delete [] decoded;
  
  for (int i = 0; i < numselectors; i++)
    delete [] table[i];
  delete [] table;

  fclose(fp);
  
  return 0;
}
