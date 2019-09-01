#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "selectorGen.h"

int SelectorGen::get_num_rows()
{
  return selected;
}

void SelectorGen::print_perm(int *permutation, int length)
{
  for (int i = 0; i < length; i++)
    printf("%d ", permutation[i]);
  printf("\n");
}

void SelectorGen::add_perm_to_table(int **table, uint row,
				    int *permutation, int length)
{
  for (int i = 0; i < length; i++)
    table[row][i] = permutation[i];
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
void SelectorGen::generate_perms(int **table, uint selected, int *x, int n,
				 void callback(int**, uint, int *, int))
{
    do
    {
      if (callback) callback(table, selected, x, n);
      selected++;
    }
    while (next_lex_perm(x, n) && selected < num_selectors);
}


void SelectorGen::generate(int **dest)
{
  for (uint i = 0; i < num_selectors; i++)
    for (uint j = 0; j < payload_bits; j++)
      dest[i][j] = 0;

  selected = 0;

  
  /* this case does occur, even in WSJ collection. Below algorithm
     doesn't deal with this correctly, so need to treat it separately
     here */
  if (rc.hst == 1)
  {
    for (int i = 0; i < payload_bits; i++)
      dest[0][i] = 1;
    
    selected = 1;
    return;
  }
  
  /* add the 28 bit selector if needed */
  if (rc.hst * 2 > payload_bits)
    dest[selected++][0] = payload_bits;

  
  /* First deal with short lists (defined as lists where you can't
     gain anything from using a combination of mode and high
     exception */
  if ((rc.md * 2 + rc.hxp) > payload_bits) 
  {
    uint start = rc.lst;
    if (start < payload_bits - rc.hst)
      start = payload_bits - rc.hst; 
    
    for (uint i = start; i <= payload_bits - start; i++)
    {
      if (selected < num_selectors)
      {
	dest[selected][0] = i;
	dest[selected++][1] = payload_bits - i;
      }
      else
	;//exit(printf("ran out of selectors, this shouldn't happen\n"));
    }
  }

  /* Catch those few cases that don't get a 2 int selector in above
     for loop but could use one */
  if (selected < 1 && rc.lst*2 < payload_bits && rc.hst*3 > payload_bits)
  {
    dest[selected][0] = payload_bits / 2;
    dest[selected][1] = payload_bits - dest[selected][0];
    selected++;
  }

  /* Different set of rules for longer lists */
  // this should be <=, not < ... fix when have time to test properly
  //if (rc.md * 2 + rc.hxp < payload_bits)
  if (rc.md * 3 <= payload_bits)
  {
    // even packing for largest numbers
    //printf("even packing for highest\n");

    int num_default_ints = payload_bits / rc.hst;
    if (selected < 1) // check if have already allowed for largest numbers in list
    {
      for (int i = 0; i < num_default_ints; i++)
      	dest[selected][i] = rc.hst;
      selected++;
    }

    // even packing for high exception
    //printf("even packing for high exception\n");
    int num_high_ints = payload_bits / rc.hxp;
    int largestpackable = payload_bits / num_high_ints;
    if (num_high_ints != num_default_ints)
    {
      for (int i = 0; i < num_high_ints; i++)
	dest[selected][i] = largestpackable;
      selected++;
    }

    // even packing for modal bitwidth
    //printf("even packing for modal bitwidth\n");
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
    
     // a combination of mode plus 25% exceptions
    int *combination = new int[payload_bits];
    for (uint i = 0; i < payload_bits; i++)
      combination[i] = 0;
    
    uint bits_available = payload_bits;
    uint i = 0;
    
    while (bits_available >= rc.hxp)
      {
	//printf("while bits_available\n");
	combination[i++] = rc.hxp;
	uint j = 0;
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

    // sort the combination, then make and count the permutations
    int comb_length = 0;
    for (uint i = 0; i < payload_bits; i++)
      {
       if (combination[i] == 0)
	break;
       comb_length++;
      }
    std::sort(combination, combination + comb_length);
    
    // then make the permutations copying to the selector table until
    // it is full or we run out of permutations
    this->generate_perms(dest, selected, combination, comb_length, add_perm_to_table);
    delete [] combination;

    // we lost the correct value of "selected" while generating perms, so find it again
    selected = 16;
    for (uint i = 0; i < num_selectors; i++)
      if (dest[i][0] == 0)
      {
	selected = i;
	break;
      }
        
    //if there is room left put in even packing of low exception
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
  //printf("generated %d selectors\n", selected);
}


void SelectorGen::sort_table(int **table)
{
  int numrows = 0;
  for (uint i = 0; i < num_selectors; i++)
  {
    int rowlength = 0;
    for (int j = 0; j < 28; j++)
      if (table[i][j] == 0)
      {
	rowlength = j;
	printf("row length: %d\n", rowlength);
	break;
      }
    if (rowlength == 0)
    {
      numrows = i;
      printf("number of rows: %d\n", numrows);
      break;
    }
  }
}


/* 
   Convert bitwidth table from 2D matrix to selector_table data
   type, while also sorting rows for optimal packing (longest rows
   first)
 */
SelectorGen::selector_table SelectorGen::convert_table(int **table)
{
  selector_table result;
    
  int numrows = get_num_rows();
  result.rows = numrows;
  result.row_lengths = new int[numrows];
  result.bitwidths = new int*[numrows];

  // get length of longest row
  // this is a problem, this can't find the 28 int packings
  int maxlength, rowlength = 0;
  int *temp_row_lengths = new int[numrows];
  // look at each row
  for (int i = 0; i < numrows; i++)
  {
    // find length of each row
    // maybe i should be recording row lengths during table generation instead.
    for (int j = 0; j < 28; j++)   ///// this '28' here is dangerous! fix this!!!
      if (table[i][j] == 0)
      {
	rowlength = j;
	if (rowlength > maxlength)
	  maxlength = rowlength;
	temp_row_lengths[i] = j;
	break;
      }
  }
  
  
  // write out rows longest to shortest
  // for each length starting with longest
  int count = 0;
  while (count < numrows)
  {
    for (int i = maxlength; i > 0; i--)
    {
      ///look at each row
      for (int j = 0; j < numrows; j++)
      {
	if (temp_row_lengths[j] == i)
        {
	  //printf("row %d has a length of %d\n", j, i);
	  result.row_lengths[count] = i;
	  result.bitwidths[count] = new int[i];
	  for (int k = 0; k < i; k++)
	    result.bitwidths[count][k] = table[j][k];
	  count++;
	}
	
      }
    }
  }

  
  //int numrows = 0;
  //int maxlength = 0;
  // for (uint i = 0; i < num_selectors; i++)
  // {
  //   int rowlength = 0;
  //   for (int j = 0; j < 28; j++)
  //     if (table[i][j] == 0)
  //     {
  // 	rowlength = j;
  // 	if (rowlength > maxlength)
  // 	  maxlength = rowlength;
  // 	temp_row_lengths[i] = j;
  // 	result.row_lengths[i] = j; // dont leave this here
  // 	break;
  //     }
  //   if (rowlength == 0)
  //   {
  //     numrows = i;
  //     printf("number of rows: %d\n", numrows);
  //     break;
  //   }
  // }

  // printf("longest row = %d\n", maxlength);
  // result.rows = numrows;
  return result;
}

void SelectorGen::print_converted_table(selector_table table)
{
  
  //printf("Selector table:\n");
  for (int i = 0; i < table.rows; i++)
  {
    printf("%d int packing: ", table.row_lengths[i]);
    for (int j = 0; j < table.row_lengths[i]; j++)
      printf("%d ", table.bitwidths[i][j]);
    printf("\n");
  }

  
  printf("\n======================\n\n");

}


void SelectorGen::print_table(int **table)
{
  for (uint i = 0; i < num_selectors; i++)
  {
    for (uint j = 0; j < payload_bits; j++)
      printf("%2d ", table[i][j]);
    printf("\n");
  }
  printf("\n");
}
