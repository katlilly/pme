#include <stdio.h>
#include <stdlib.h>
#include "selectorGen.h"


void SelectorGen::generate(int **dest)
{
  for (int i = 0; i < num_selectors; i++)
    for (int j = 0; j < payload_bits; j++)
      dest[i][j] = 0;

  int selected = 0;

  /* add the 28 bit selector if needed */
  if (rc.hst * 2 > payload_bits)
    dest[selected++][0] = payload_bits;

  /* First deal with short lists (defined as lists where you can't
     gain anything from using a combination of mode and high
     exception */
  if ((rc.md * 2 + rc.hxp) > payload_bits) 
  {
    int start = rc.lst;
    if (start < payload_bits - rc.hst)
      start = payload_bits - rc.hst; 
    
    for (int i = start; i <= payload_bits - start; i++)
    {
      if (selected < num_selectors)
      {
	dest[selected][0] = i;
	dest[selected++][1] = payload_bits - i;
      }
      else
      	exit(printf("ran out of selectors, this shouldn't happen\n"));
    }
  }

  /* catch those few cases that don't get a 2 int selector in above
     for loop but could use one */
  if (selected < 1 && rc.lst*2 < payload_bits && rc.hst*3 > payload_bits)
  {
    dest[selected][0] = payload_bits / 2;
    dest[selected][1] = payload_bits - dest[selected][0];
    selected++;
  }

  /* Different set of rules for longer lists */
  if (rc.md * 2 + rc.hxp < payload_bits)
  {
    //printf("now is the time for permutations of mode and highexcp\n");

    // first an even packing of each of highest, highexcp, and mode
    // or, first pack three evenly, continuing up to x where x is payload_bits / lowest

    int num_default_ints = payload_bits / rc.hst;
    if (selected < 1)
    {
      for (int i = 0; i < num_default_ints; i++)
      {
	dest[selected][i] = rc.hst;
      }
      selected++;
    }

    int num_high_ints = payload_bits / rc.hxp;
    if (num_high_ints != num_default_ints)
    {
      for (int i = 0; i < num_high_ints; i++)
	dest[selected][i] = rc.hxp;
      selected++;
    }

    /// to do convert to highest possible number for same number of ints packed...
    /// eg 8 8 8 should be 9 9 9
    /// but don't let this mess with the way i do permutations of mode and exceptions
    
    int num_mode_ints = payload_bits / rc.md;
    if (num_mode_ints != num_high_ints)
    {
      for (int i = 0; i < num_mode_ints; i++)
	dest[selected][i] = rc.md;
      selected++;
    }
    

    int num_low_ints = payload_bits / rc.lst;
    if (num_low_ints != num_mode_ints)
    {
      for (int i = 0; i < num_low_ints; i++)
	dest[selected][i] = rc.lst;
      selected++;
    }

    // then one or more exceptions with mode, and permutations thereof
    // keep adding permutations until selectors are all used up
    
    
  }

  

  
}



void SelectorGen::print_table(int **table)
{
  
  for (int i = 0; i < num_selectors; i++)
  {
    for (int j = 0; j < payload_bits; j++)
      printf("%2d ", table[i][j]);
    printf("\n");
  }
  printf("\n");
}
