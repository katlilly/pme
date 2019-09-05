#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "selectorGen.h"

/* 
   Get the number of bits used for the selector. This is always set by
   the constructor
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
   this list. The number of rows in the generated table
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
    int sum = 0;
    printf("%d int packing: ", table.row_lengths[i]);
    for (int j = 0; j < table.row_lengths[i]; j++)
    {
      sum += table.bitwidths[i][j];
      printf("%d ", table.bitwidths[i][j]);
    }
    printf(" = %d bits used\n", sum);
    if (sum > payload_bits)
      printf("******************* exceedes payload size ****************\n");
    //if (sum <= payload_bits - mode)
    //printf("******************* arguably underful payload ************\n");
    if (sum <= payload_bits - table.bitwidths[i][0])
      exit(printf("******************* underful payload ************\n"));
	
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

/* 
   Generate selector table for the rare case where all or 90% of
   the dgaps are ones
*/
int SelectorGen::all_ones(selector_table *table)
{
  //printf("list number: %d\n", listnumber);
  if (highest == 1)
    {
      //printf("listnumber = %d, highest == 1\n", listnumber);
      table->rows = 1;
      selected = 1;
    }
    else
    {
      // actually i can do better than this
      table->rows = 2;
      selected = 2;
    }

    table->row_lengths = new int[table->rows];
    table->row_lengths[0] = payload_bits;
    table->bitwidths = new int*[table->rows];
    table->bitwidths[0] = new int[payload_bits];
    for (int i = 0; i < payload_bits; i++)
      table->bitwidths[0][i] = 1;

    if (highest > 1)
    {
      // should also be including permutations of ones and highest
      table->bitwidths[1] = new int[1];
      table->bitwidths[1][0] = payload_bits;
      table->row_lengths[1] = 1;
    }
    return table->rows;
}

/*
  Add the full-payload-size selector if needed
 */
void SelectorGen::add_one_int_selector(selector_table *table, int row)
{
  table->row_lengths[row] = 1;
  table->bitwidths[row] = new int[1];
  table->bitwidths[row][0] = payload_bits;
  table->rows = row + 1;
}


int SelectorGen::add_two_dgap_selectors(selector_table *table, int row)
{
  uint start = lowest;
  int count = 0;
  if (start < payload_bits - highest)
    start = payload_bits - highest; 
    
  for (uint i = start; i <= payload_bits - start; i++)
    if (row < get_num_selectors())
      {
	table->row_lengths[row] = 2;
	table->bitwidths[row] = new int[2];
	table->bitwidths[row][0] = i;
	table->bitwidths[row++][1] = payload_bits - i;
	table->rows++;
	count++;
      }

  return count;
}

/* 
   Even packings for largest and modal dgaps
*/
int SelectorGen::pack_largest(selector_table *table, int row)
{
  int count = 0;

  // even packing for largest numbers
  int num_default_ints = payload_bits / highest;
  if (row < 1) // this checks if largest dgaps have already been accomodated
  {
    table->row_lengths[row] = num_default_ints;
    table->bitwidths[row] = new int[num_default_ints];
    for (int i = 0; i < num_default_ints; i++)
      table->bitwidths[row][i] = highest;
    table->rows++;
    count++;
    row++;
  }

  // even packing for high exception
  int num_high_ints = payload_bits / high_exception;
  int largestpackable_hxp = payload_bits / num_high_ints;
  if (num_high_ints != num_default_ints)
  {
    table->row_lengths[row] = num_high_ints;
    table->bitwidths[row] = new int[num_high_ints];
    for (int i = 0; i < num_high_ints; i++)
      table->bitwidths[row][i] = largestpackable_hxp;
    table->rows++;
    count++;
    row++;
  }

  // even packing for modal bitwidth
  int num_mode_ints = payload_bits / mode;
  int largestpackable_md = payload_bits / num_mode_ints;
  if (num_mode_ints != num_high_ints)
  {
    table->row_lengths[row] = num_mode_ints;
    table->bitwidths[row] = new int[num_mode_ints];
    for (int i = 0; i < num_mode_ints; i++)
      table->bitwidths[row][i] = largestpackable_md;
    table->rows++;
    count++;
  }

  return count;
}


int SelectorGen::add_permutations(selector_table *table, int row)
{
  //printf("entered add_permutations()\n");
  
  /*
    Generate the most useful combination of bitwidths
   */
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

  int comb_length = 0;
  for (uint i = 0; i < payload_bits; i++)
    {
      if (combination[i] == 0)
	break;
      comb_length++;
    }
  std::sort(combination, combination + comb_length);
  

  /* Now make the permutations of that combination, copying to the
     selector table until it is full or we run out of
     permutations */
  this->generate_perms(table, table->rows, combination, comb_length, add_perm_to_table);
  delete [] combination;
  
  return num_selectors - row;
}

int SelectorGen::add_other_two_dgap_selector(selector_table *table, int row)
{
  table->row_lengths[row] = 2;
  table->bitwidths[row] = new int[2];
  table->bitwidths[row][0] = payload_bits / 2;
  table->bitwidths[row][1] = payload_bits - table->bitwidths[row][0];
  table->rows++;
  //printf("added a 2 int selector by the less common method, %d rows\n", 1);

  return 1;
}


int SelectorGen::add_low_exception(selector_table *table, int row)
{
  int num_low_ints = payload_bits / lowest;
  table->row_lengths[row] = num_low_ints;
  table->bitwidths[row] = new int[num_low_ints];
  for (int i = 0; i < num_low_ints; i++)
    table->bitwidths[row][i] = lowest;
    
  table->rows++;
  return 1;
}

void SelectorGen::smart_generate(selector_table *table)
{
  int row = 0;

  /* 
     Deal with lists where more than 90% of dgaps are 1. Can return
     directly after this if true
  */
  if (high_exception == 1)
  {
    row = all_ones(table);
    return;
  }
  
  /* 
     Add the full payload size "28 bit" selector if needed
  */
  if (highest * 2 > payload_bits)
    add_one_int_selector(table, row++);

  /* 
     Now deal with short lists (defined as lists where you can't
     gain anything from using a combination of mode and high
     exception)
  */
  if ((mode * 2 + high_exception) > payload_bits) 
     row += add_two_dgap_selectors(table, row);

  /*
     Catch those few cases that don't get a 2 int selector in above
     for loop but could use one
  */
  if (row < 1 &&
      lowest * 2 < payload_bits &&
      highest * 3 > payload_bits)
    row += add_other_two_dgap_selector(table, row);

  /* 
     Even packing for largest numbers and high exception and mode.
     Used for lists long enough that we can pack at least three dgaps
     into one payload
   */
  if (mode * 3 <= payload_bits)
    row += pack_largest(table, row);

  /****** 
       Need empirical data on whether low exception row should come
       before permutations
  *******/
 
  /* 
    Add an even packing of the low exception
  */
    if (selected < num_selectors)
      row += add_low_exception(table, row);
      
   /*
    Add permutations of mode and high exception
   */
  if (mode < (payload_bits / 4))
    row += add_permutations(table, row);

  printf("%d rows generated so far\n", row);
}


/* 
   Add a new permutation of bitwidths to the selector table
*/
void SelectorGen::add_perm_to_table(selector_table *table, uint row, int *permutation, int length)
{
  table->bitwidths[row] = new int[length];
  table->row_lengths[row] = length;
  //printf("created a new selector row of length %d\n", length);
  for (int i = 0; i < length; i++)
    {
      //printf("%d ", permutation[i]);
      
      table->bitwidths[row][i] = permutation[i];
      //table[row][i] = permutation[i];
    }
  table->rows++;

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
void SelectorGen::generate_perms(selector_table *table, uint selected, int *x, int n, void callback(selector_table*, uint, int *, int))
{
    do
    {
      if (callback)
	{
	  callback(table, table->rows, x, n);
	  //callback(table, selected, x, n);
	}
      selected++;
    }
    while (next_lex_perm(x, n) && table->rows < num_selectors);
    //while (next_lex_perm(x, n) && selected < num_selectors);

}
