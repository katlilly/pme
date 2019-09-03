#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "selectorGen.h"

/* 
   Get the number of bits used for the selector. This is set by the constructor
*/
int SelectorGen::get_selector_size()
{
  return selector_bits;
}

/*
  Get the maximum number of selectors. Note this is the amount that can be
  described by the selector bits, not the actual amount used, see
  get_num_rows()
*/
int SelectorGen::get_num_selectors()
{
  int result = 1;
  for (int i = 0; i < selector_bits; i++)
    result *= 2;
  return result;
}

/* 
   Get the number of selectors generated for the selector table for
   this list. The number of rows in the table
*/
int SelectorGen::get_num_rows()
{
  return selected;
}

/* 
   Print the selector table
*/
void SelectorGen::print_table(selector_table table)
{
  for (int i = 0; i < table.rows; i++)
  {
    printf("%d int packing: ", table.row_lengths[i]);
    for (int j = 0; j < table.row_lengths[i]; j++)
      printf("%d ", table.bitwidths[i][j]);
    printf("\n");
  }
  printf("\n=============================\n\n");
}

/* 
   Print the statistics for the current list
*/
void SelectorGen::print_stats(void)
{
  printf("Lowest: %d\n", lowest);
  printf("Mode: %d\n", mode);
  printf("High exception: %d\n", high_exception);
  printf("Highest: %d\n\n", highest);
 return;
}

void SelectorGen::generate(selector_table *table)
{
  /* Create and initialise a temporary matrix for use in creating the
     selector table, and a list of row lengths*/
  int *temp_row_lengths = new int[selected];
  int max_rows = get_num_selectors();
  int** temptable = new int*[max_rows];
  for (int i = 0; i < max_rows; i++)
  {
    temp_row_lengths[i] = 0;
    temptable[i] = new int[payload_bits];
    for (int j = 0; j < payload_bits; j++)
      temptable[i][j] = 0;
  }
  selected = 0;
  
  /* this case (where largest number in list is 1) does occur, even in
     WSJ collection. Below algorithm doesn't deal with this correctly,
     so need to treat it separately here */
  // this should come before creating temp matrix /////////////////////////////////////
  if (highest == 1)
  {
    table->rows = 1;
    table->row_lengths = new int[1];
    table->row_lengths[0] = payload_bits;
    table->bitwidths = new int*[1];
    table->bitwidths[0] = new int[payload_bits];
    for (int i = 0; i < payload_bits; i++)
      table->bitwidths[0][i] = 1;
    //temp_row_lengths[selected] = payload_bits;
    selected = 1;
    
    return;
  }

 /* add the full size "28 bit" selector if needed */
  if (highest * 2 > payload_bits)
  {
    temp_row_lengths[selected] = 1;
    temptable[selected++][0] = payload_bits;
  }
  
/* First deal with short lists (defined as lists where you can't
     gain anything from using a combination of mode and high
     exception) */
  if ((mode * 2 + high_exception) > payload_bits) 
  {
    uint start = lowest;
    if (start < payload_bits - highest)
      start = payload_bits - highest; 
    
    for (uint i = start; i <= payload_bits - start; i++)
      if (selected < num_selectors)
      {
	temp_row_lengths[selected] = 2;
	temptable[selected][0] = i;
	temptable[selected++][1] = payload_bits - i;
      }
      else
	;//exit(printf("ran out of selectors, this shouldn't happen\n"));
  }
  
  /*
    Catch those few cases that don't get a 2 int selector in above
     for loop but could use one
  */
  if (selected < 1 &&
      lowest * 2 < payload_bits &&
      highest * 3 > payload_bits)
  {
    temp_row_lengths[selected] = 2;
    temptable[selected][0] = payload_bits / 2;
    temptable[selected][1] = payload_bits - temptable[selected][0];
    selected++;
  }

/* Different set of rules for longer lists */
  // this should be <=, not <  ...fix when have time to test properly
  //if (rc.md * 2 + rc.hxp < payload_bits)
  if (mode * 3 <= payload_bits)
  {
    // even packing for largest numbers
    int num_default_ints = payload_bits / highest;
    if (selected < 1) // this checks if largest numbers have already been allowed for
    {
      for (int i = 0; i < num_default_ints; i++)
      {
	temp_row_lengths[selected] = num_default_ints;
      	temptable[selected][i] = highest;
      }
      selected++;
    }

    // even packing for high exception
    int num_high_ints = payload_bits / high_exception;
    int largestpackable = payload_bits / num_high_ints;
    if (num_high_ints != num_default_ints)
    {
      for (int i = 0; i < num_high_ints; i++)
      {
	temp_row_lengths[selected] = num_high_ints;
	temptable[selected][i] = largestpackable;
      }
      selected++;
    }

    // even packing for modal bitwidth
    int num_mode_ints = payload_bits / mode;
    largestpackable = payload_bits / num_mode_ints;
    if (num_mode_ints != num_high_ints)
    {
      for (int i = 0; i < num_mode_ints; i++)
      {
	temp_row_lengths[selected] = num_mode_ints;
	temptable[selected][i] = largestpackable;
      }
      selected++;
    }
    
    /* Generate the most useful bitwidth combination. The (unweighted)
       mean exception frequency of lists where mode+mode+highexp < 28
       is 0.24, so I am using a combination of mode plus 25%
       exceptions */
    /// perhaps should move this to a separate function
    int *combination = new int[payload_bits];
    for (uint i = 0; i < payload_bits; i++)
      combination[i] = 0;
    
    uint bits_available = payload_bits;
    uint i = 0;
    
    while (bits_available >= high_exception)
      {
	combination[i++] = high_exception;
	uint j = 0;
	bits_available -= high_exception;
	while (bits_available >= mode && j < 3)
	  {
	    combination[i++] = mode;
	    j++;
	    bits_available -= mode;
	  }
      }
    if (bits_available >= mode)
      combination[i] = mode;

    /* Sort the combination, then make and count the permutations */
    int comb_length = 0;
    for (uint i = 0; i < payload_bits; i++)
      {
	if (combination[i] == 0)
	  break;
	comb_length++;
      }
    std::sort(combination, combination + comb_length);

    /* add row lengths in advance for the permutations */
    for (int i = 0; i < num_selectors - selected; i++)
      temp_row_lengths[selected + i] = comb_length;
    /* above will sometimes add lengths at the end that aren't
     correct, but these will always be either overwritten or
     ignored. I can't know how many permutations there will be until i
     have counted them */
    
    /* Now make the permutations of that combination, copying to the
       selector table until it is full or we run out of
       permutations */
    this->generate_perms(temptable, selected, combination, comb_length, add_perm_to_table);
    delete [] combination;

    /* We lost the correct value of "selected" while generating perms,
       so find it again here */
    // this does give correct answer in all cases, but think about if
    // I can do this better - should there be a record of this number
    // with the class somewhere??
    selected = 16;
    for (uint i = 0; i < num_selectors; i++)
      if (temptable[i][0] == 0)
	{
	  selected = i;
	  break;
	}

    
    /* If there is room left in selector table add in an even packing
       of the low exception */
    if (selected < num_selectors)
      {
	int num_low_ints = payload_bits / lowest;
	if (num_low_ints != num_mode_ints)
	  {
	    for (int i = 0; i < num_low_ints; i++)
	      temptable[selected][i] = lowest;
	    selected++;
	  }
      }
  }


  /* now we have the 2d matrix version, convert it to a proper selector table*/
  table->rows = selected;
  table->row_lengths = new int[selected];
  table->bitwidths = new int*[selected];

  /* get length of longest row */
  int maxlength = 0;
  for (int i = 0; i < selected; i++)
    if (temp_row_lengths[i] > maxlength)
      maxlength = temp_row_lengths[i];

  printf("max ints per row: %d\n", maxlength);
  printf("number of selectors: %d\n\n", selected);

  /* write out selectors and their lengths, longest to shortest */
  int count = 0;
  while (count < selected)
    for (int i = maxlength; i > 0; i--)
      for (int j = 0; j < selected; j++)
	if (temp_row_lengths[j] == i)
	{
	  table->row_lengths[count] = i;
	  table->bitwidths[count] = new int[i];
	  for (int k = 0; k < i; k++)
	    table->bitwidths[count][k] = temptable[j][k];
	  count++;
	}
  


  
  
  //for (int i= 0; i < selected; i++)
  //delete [] temptable[i];
  //delete [] temptable;

  delete [] temp_row_lengths;
  
  return;
}


/* 
   Add a new permutation of bitwidths to the selector table
*/
void SelectorGen::add_perm_to_table(int **table, uint row, int *permutation, int length)
{
  for (int i = 0; i < length; i++)
    table[row][i] = permutation[i];
}

/* 
   Get next lexicographical permutation. taken from rosettacode
*/
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

/* 
   Generates permutations in correct order and outputs unique ones -
   taken directly from rosettacode
*/
void SelectorGen::generate_perms(int **table, uint selected, int *x, int n, void callback(int**, uint, int *, int))
{
    do
    {
      if (callback)
	callback(table, selected, x, n);
      selected++;
    }
    while (next_lex_perm(x, n) && selected < num_selectors);
}
