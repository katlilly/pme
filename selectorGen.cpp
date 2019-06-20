#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "selectorGen.h"

void SelectorGen::add_perm_to_table(int *permutation, int length)
{
    for (int i = 0; i < length; i++)
    {
      table[selected][i] = permutation[i];
    }
}

/* get next lexicographical permutation. taken from rosettacode */
int SelectorGen::next_lex_perm(int *a, int n) {
#define swap(i, j) {t = a[i]; a[i] = a[j]; a[j] = t;}
    int k, l, t;
    
    /* 1. Find the largest index k such that a[k] < a[k + 1]. If no such
     index exists, the permutation is the last permutation. */
    for (k = n - 1; k && a[k - 1] >= a[k]; k--);
    
    if (!k--) return 0;
    /* 2. Find the largest index l such that a[k] < a[l]. Since k + 1 is
     such an index, l is well defined */
    for (l = n - 1; a[l] <= a[k]; l--) {
        ;
    }
    
    /* 3. Swap a[k] with a[l] */
    swap(k, l);
    
    /* 4. Reverse the sequence from a[k + 1] to the end */
    for (k++, l = n - 1; l > k; l--, k++)
        swap(k, l);
    return 1;
#undef swap
}


/* generates permutations in correct order and outputs unique ones
 - taken directly from rosettacode */
void SelectorGen::generate_perms(int *x, int n, void callback(int *, int))
{
    do
    {
      if (callback) callback(x, n);
      selected++;
    }
    while (next_lex_perm(x, n) && selected < num_selectors);
}


void SelectorGen::generate(int **dest)
{
  for (int i = 0; i < num_selectors; i++)
    for (int j = 0; j < payload_bits; j++)
      dest[i][j] = 0;

  selected = 0;

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

  // this should be <=, not < ... fix when have time to test properly
  /* Different set of rules for longer lists */
  //if (rc.md * 2 + rc.hxp < payload_bits)
  if (rc.md * 3 < payload_bits)
  {
    //printf("now is the time for permutations of mode and highexcp\n");

    // first an even packing of each of highest, highexcp, and mode
    // or, first pack three evenly, continuing up to x where x is payload_bits / lowest

    // even packing for largest numbers
    int num_default_ints = payload_bits / rc.hst;
    if (selected < 1) // check if have already allowed for largest numbers in list
    {
      for (int i = 0; i < num_default_ints; i++)
      	dest[selected][i] = rc.hst;
      selected++;
    }

    // even packing for high exception
    int num_high_ints = payload_bits / rc.hxp;
    int largestpackable = payload_bits / num_high_ints;
    if (num_high_ints != num_default_ints)
    {
      for (int i = 0; i < num_high_ints; i++)
	dest[selected][i] = largestpackable;
      selected++;
    }

    // even packing for modal bitwidth
    int num_mode_ints = payload_bits / rc.md;
    largestpackable = payload_bits / num_mode_ints;
    if (num_mode_ints != num_high_ints)
    {
      for (int i = 0; i < num_mode_ints; i++)
	dest[selected][i] = largestpackable;
      selected++;
    }
    
    /* the (unweighted) mean exception frequency of lists where
       mode+mode+highexp < 28 is 0.24 */
    
    // a combination for mode plus not less than 25% exceptions
    int *combination = new int[payload_bits];
    for (int i = 0; i < payload_bits; i++)
      combination[i] = 0;
    int bits_available = payload_bits;
    int i = 0;
    while (bits_available >= rc.hxp)
      {
	combination[i++] = rc.hxp;
	int j = 0;
	bits_available -= rc.hxp;
	while (bits_available >= rc.md && j < 3)
	{
	  combination[i++] = rc.md;
	  j++;
	  bits_available -= rc.md;
	}
      }
    if (bits_available >= rc.md)
      combination[i] = rc.md;

    // next sort the combination, then make and count the permutations
    int comb_length = 0;
    for (int i = 0; i < payload_bits; i++)
      {
       if (combination[i] == 0)
	break;
       comb_length++;
      }
    std::sort(combination, combination + comb_length);

    
    // then make the permutations copying to the selector table until
    // it is full or we run out of permutations
    //generator.generate_perms(combination, comb_length, add_perm_to_table);




    // if there is room left put this one in (or possible all the even
    // packings below mode...
    if (selected < num_selectors)
    {
      int num_low_ints = payload_bits / rc.lst;
      if (num_low_ints != num_mode_ints)
      {
	for (int i = 0; i < num_low_ints; i++)
	  dest[selected][i] = rc.lst;
	selected++;
      }
    }
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
