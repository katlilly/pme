#include <stdio.h>
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
  Get the maximum number of selectors. This is the amount that can be
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
   Return the size of the payload in bits
 */
int SelectorGen::get_payload_bits()
{
  return payload_bits;
}

/*
  Generate the selector table for the current list
 */
void SelectorGen::generate(selector_table *table)
{
  uint row = 0;

  /* 
     Create selector table for lists where more than 90% of dgaps are 1
  */
  if (high_exception == 1)
  {
    row = all_ones(table);
    return;
  }
  
  /* 
     Add the single-dgap, full-payload-size selector if needed
  */
  if (highest * 2 > payload_bits)
    add_one_int_selector(table, row++);
  
  /* 
     Add two dgap selectors for short lists (defined as lists where
     you can't gain anything from using a combination of mode and high
     exception)
  */
  if ((mode * 2 + high_exception) > payload_bits) 
     row += add_two_dgap_selectors(table, row);
  
  /*
     Catch those few cases that don't get a 2 int selector in above
     for loop but could use one.
  */
  if (row < 1 && lowest * 2 < payload_bits && highest * 3 > payload_bits)
    row += add_other_two_dgap_selector(table, row);

  /* 
     Add even packings for largest dgaps and for high exception and for
     mode.
   */
  if (mode * 3 <= payload_bits)
    row += add_even_packings(table, row);
  
  /*
    Add permutations of mode and high exception.
  */
  if (mode < (payload_bits / 4))
    row = add_permutations(table, row);

  /* 
    Add an even packing of the low exception if row(s) remaining.
  */
  if (row < num_selectors && lowest <= payload_bits / 3)
    row += add_low_exception(table, row);
  
}

/* 
   Print the selector table
*/
void SelectorGen::print_table(selector_table table)
{
  for (uint i = 0; i < table.rows; i++)
  {
    uint sum = 0;
    printf("%2d int packing: ", table.row_lengths[i]);
    for (int j = 0; j < table.row_lengths[i]; j++)
    {
      sum += table.bitwidths[i][j];
      printf("%d ", table.bitwidths[i][j]);
    }
    printf(" = %d bits used\n", sum);
    
    if (sum > payload_bits)
      exit(printf("exceedes payload size\n"));
    if (sum <= payload_bits - table.bitwidths[i][0])
      exit(printf("underful payload\n"));
  }
  printf("\n================================\n\n");
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
   Generate selector table for the case where all or 90% of the dgaps
   are ones
*/
int SelectorGen::all_ones(selector_table *table)
{
  if (highest == 1)
    {
      table->rows = 1;
      selected = 1;
    }
    else
    {
      table->rows = 2;
      selected = 2;
    }

    table->row_lengths[0] = payload_bits;

    for (uint i = 0; i < payload_bits; i++)
      table->bitwidths[0][i] = 1;

    if (highest > 1)
    {
      // probably should also be including permutations of ones and highest
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
  table->bitwidths[row][0] = payload_bits;
  table->rows = row + 1;
}

/*
  Add all the usable two-dgap selectors
 */
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
int SelectorGen::add_even_packings(selector_table *table, int row)
{
  int count = 0;

  // even packing for largest numbers
  int num_default_ints = payload_bits / highest;
  if (row < 1) // this checks if largest dgaps have already been accomodated
  {
    table->row_lengths[row] = num_default_ints;
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
    for (int i = 0; i < num_mode_ints; i++)
      table->bitwidths[row][i] = largestpackable_md;
    table->rows++;
    count++;
  }

  return count;
}

/*
  Generate the most useful combination of bitwidths, and add as many
  permutations of that as will fit in the selector table
 */
int SelectorGen::add_permutations(selector_table *table, int row)
{ 
  int *combination = new int[payload_bits];
  for (uint i = 0; i < payload_bits; i++)
    combination[i] = 0;

  uint bits_available = payload_bits;
  uint i = 0;
    
  while (bits_available >= high_exception)
  {
    // add one high exception
    combination[i++] = high_exception;
    bits_available -= high_exception;

    // add three modes
    uint j = 0;
    while (bits_available >= mode && j < 3)
    {
      combination[i++] = mode;
      j++;
      bits_available -= mode;
    }
  }

  // fill any remaining payload with modal bitwidths
  while (bits_available >= mode && i < payload_bits)
  {
    combination[i++] = mode;
    bits_available -= mode;
  }
 
  /* 
     Create the permutations of that combination, copying to the
     selector table until it is full or we run out of permutations
  */
  std::sort(combination, combination + i);
  this->generate_perms(table, table->rows, combination, i, add_perm_to_table);

  delete [] combination;
  return table->rows;
}

/*
  Add a two dgap selector for those that didn't get one the usual way
*/
int SelectorGen::add_other_two_dgap_selector(selector_table *table, int row)
{
  table->row_lengths[row] = 2;
  table->bitwidths[row][0] = payload_bits / 2;
  table->bitwidths[row][1] = payload_bits - table->bitwidths[row][0];
  table->rows++;

  return 1;
}

/*
  Add even packing of low exception to selector table
*/
int SelectorGen::add_low_exception(selector_table *table, int row)
{
  int num_low_ints = payload_bits / lowest;
  int num_mode_ints = payload_bits / mode;
  if (num_low_ints == num_mode_ints)
    return 0;
  table->row_lengths[row] = num_low_ints;
  for (int i = 0; i < num_low_ints; i++)
    table->bitwidths[row][i] = lowest;
    
  table->rows++;
  return 1;
}

/* 
   Generates permutations in correct order and outputs unique ones to
   my selector table. Taken from rosettacode
*/
void SelectorGen::generate_perms(selector_table *table, uint selected, int *x, int n,
				 void callback(selector_table*, uint, int *, int))
{
  do
  {
    if (callback)
      callback(table, table->rows, x, n);
    selected++;
  }
  while (next_lex_perm(x, n) && table->rows < num_selectors);
}

/* 
   Add a new permutation of bitwidths to the selector table
*/
void SelectorGen::add_perm_to_table(selector_table *table, uint row, int *permutation,
				    int length)
{
  //table->bitwidths[row] = new int[length];
  table->row_lengths[row] = length;
  for (int i = 0; i < length; i++)
      table->bitwidths[row][i] = permutation[i];
  table->rows++;
}

/* 
   Get the next in order permutation. Taken from rosettacode
*/
int SelectorGen::next_lex_perm(int *a, int n) {
#define swap(i, j) {t = a[i]; a[i] = a[j]; a[j] = t;}
  int k, l, t;
    
  /* Find the largest index k such that a[k] < a[k + 1]. If no such
     index exists, the permutation is the last permutation. */
  for (k = n - 1; k && a[k - 1] >= a[k]; k--);
  if (!k--)
    return 0;
  
  /* Find the largest index l such that a[k] < a[l]. Since k + 1 is
     such an index, l is well defined */
  for (l = n - 1; a[l] <= a[k]; l--)
    ;
  
  /* Swap a[k] with a[l] */
  swap(k, l);
  
  /* Reverse the sequence from a[k + 1] to the end */
  for (k++, l = n - 1; l > k; l--, k++)
    swap(k, l);
  return 1;
#undef swap
}
