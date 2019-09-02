#pragma once

class SelectorGen
{
 public:
  int selector_bits;
    
  struct record {
    uint lst;
    uint md;
    uint hxp;
    uint hst;
  };
  record rc;

  struct selector_table {
    int rows;
    int *row_lengths;
    int **bitwidths;
  };
  //selector_table st;
  

 private:
  uint num_selectors = 1;
  uint payload_bits;
  uint selected;
  
 public:

  /* 
     Initialised with the precalculated statistics of a list, as well
     as the number of bits we are chosing to use for the selector in
     the current experiment.
   */
  SelectorGen(int selector_bits_in, uint lowest, uint mode, uint highexp, uint highest)
    {
      selector_bits = selector_bits_in;
      payload_bits = 32 - selector_bits;

      for (int i = 0; i < selector_bits; i++)
	num_selectors *= 2;

      rc.lst = lowest;
      rc.md = mode;
      rc.hxp = highexp;
      rc.hst = highest;
      
    }
  
  
 public:

  /*
    Generate a selector table for the current list. Each list gets a
    bespoke selector table based on the known bitwidth statistics of
    that list and the chosen size of the table (how many bits we chose
    to set aside for the selector).
   */
  void generate(int **dest);

  /*
    Check how many rows have been used in selector. Useful for
    selector size vs compressed data size experiments.
   */
  int get_num_rows();

  /*
    Print the selector table to screen. for error checking.
   */
  void print_table(int **table);


  void print_converted_table(selector_table table);
  
  selector_table convert_table(int **table);

  int get_selector_size();

  int get_num_selectors();

  
 private:
  
  /*
    Generate permutations of a row of bitwidths
   */
  void generate_perms(int **table, uint row, int *x, int n, void callback(int**, uint, int *, int));

  /*
    Find the next in-order permutation of a row of bitwidths
   */
  int next_lex_perm(int *a, int n);

  /*
    Add a row to the selector table using the current permutation,
    used as callback by generate_perms function
   */
  static void add_perm_to_table(int **table, uint row, int *permutation, int length);


};
