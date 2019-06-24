#pragma once

class SelectorGen
{
 public:
  int selector_bits;
    
  struct record {
    uint lst;
    uint hst;
    uint hxp;
    uint md;
  };
  record rc;

 private:
  uint num_selectors = 1;
  uint payload_bits;
  uint selected;
  
 public:
  SelectorGen(int selector_bits_in, uint lowest, uint highest, uint highexp, uint mode)
    {
      selector_bits = selector_bits_in;
      payload_bits = 32 - selector_bits;

      for (int i = 0; i < selector_bits; i++)
	num_selectors *= 2;

      rc.lst = lowest;
      rc.hst = highest;
      rc.hxp = highexp;
      rc.md = mode;
    }
  
 public:
  void print_table(int **table);
  void generate(int **dest);
  int get_num_rows();

 private:
  void generate_perms(int **table, uint row, int *x, int n, void callback(int**, uint, int *, int));
  int next_lex_perm(int *a, int n);
  void add_perm_to_table(uint *row, int length);
  static void print_perm(int *permutation, int length);
  static void add_perm_to_table(int **table, uint row, int *permutation, int length);



};
