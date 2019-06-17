#pragma once

class ListStats
{

 public:

  int listNumber;
  int listLength;

  double mean;
  double stdev;

  int mode;
  int highexcp;
  int lowexcp;
  int lowest;
  int highest;

  double modFrac;
  double lowFrac;
  double highFrac;

  int range;
  int totalrange;

 public:
  ListStats(int listID, int length)
    {
      listNumber = listID;
      listLength = length;
    }

  
 public:
  
  /* 
     calculate statistics of a list for use in selector generator
   */
  void calculate_stats(const int *bitwidths, int length);
  void docnums_to_dgaps(int *dest, int *source, int length);
  void dgaps_to_bitwidths(int *dest, int *source, int length);
  void docnums_to_dgap_bitwidths(int *dest, int *source, int length);
  void print_stats(void);
  void print_stats_short(void);
  void encode_stats(uint *dest);
  void decode_stats(uint *encoded);

  
  //void get_stats();

  /* 
     Generates permutations in correct order and outputs unique ones
     *Taken from Rosetta Code*
     I've been calling this in main, perhaps should be part of getstats??
   */
  //void generate_perms(int *x, int n, void callback(int *, int));

  /* 
     
   */
  //int *make_combs(void);

 private:
  //int next_lex_perm(int *a, int n);
  //void output_perms(int *array, int length);


};
