#pragma once

class SelectorGen
{
 public:
  int listnumber;
  uint lowest;
  uint mode;
  uint high_exception;
  uint highest;

  struct selector_table {
    int rows;
    int *row_lengths;
    int **bitwidths;
  };
 
 private:
  int selector_bits;
  uint num_selectors;
  uint payload_bits;
  uint selected;
  
 public:
  /* 
     Initialised with the precalculated statistics of a list, as well
     as the number of bits we are chosing to use for the selector in
     the current experiment.
   */
  SelectorGen(int selector_bits_in, int list_num, uint encoded_stats) 
    {
      selector_bits = selector_bits_in;
      listnumber = list_num;
      payload_bits = 32 - selector_bits;
      num_selectors = 1;
      for (int i = 0; i < selector_bits; i++)
	num_selectors *= 2;

      uint code = encoded_stats;
      lowest = code & 0xff;
      code = code >> 8;
      highest = code & 0xff;
      code = code >> 8;
      high_exception = code & 0xff;
      code = code >> 8;
      mode = code & 0xff;
    }
  
 public:
  /*
    Generate a selector table for the current list. Each list gets a
    bespoke selector table based on the known bitwidth statistics of
    that list and the chosen size of the table (how many bits we chose
    to set aside for the selector).
  */
  void generate(selector_table *table);

  
  /* 
     Get the number of bits to be used for the selector. This is set by the
     constructor
  */
  int get_selector_size();

  /*
    Get the maximum number of selectors. Note this is the amount that
    can be described by the selector bits, not the actual amount used,
    see get_num_rows()
  */
  int get_num_selectors();

  /*
    Check how many rows have been used in selector. Useful for
    selector size vs compressed data size experiments.
  */
  int get_num_rows();
  
  /*
    Return the size of the payload in bits
  */
  int get_payload_bits();

  /*
    Print the selector table to screen. for error checking.
  */
  void print_table(selector_table table);

  /*
    Print the list statistics to screen. for error checking.
  */
  void print_stats(void);
  
 private:

  /* below functions called by smart_generator() and add one or more
     rows to the selector table */
  int all_ones(selector_table *table);
  void add_one_int_selector(selector_table *table, int row);
  int add_two_dgap_selectors(selector_table *table, int row);
  int add_other_two_dgap_selector(selector_table *table, int row);
  int add_even_packings(selector_table *table, int row);
  int add_permutations(selector_table *table, int row);
  int add_low_exception(selector_table *table, int row);

  /*
    Generate permutations of a row of bitwidths
   */
  void generate_perms(selector_table *table, uint row, int *x, int n,
		      void callback(selector_table*, uint, int *, int));

  /*
    Find the next in-order permutation of a row of bitwidths
   */
  int next_lex_perm(int *a, int n);

  /*
    Add a row to the selector table using the current permutation,
    used as callback by generate_perms function
   */
  static void add_perm_to_table(selector_table *table, uint row, int *permutation,
				int length);

};
