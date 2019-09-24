#pragma once

class ListStats
{
 public:

  int listNumber;
  int listLength;

  double mean;   // mean bitwidth of d-gaps
  double stdev;  // of above

  int mode;      // most common bitwidth of d-gaps
  int highexcp;  // bitwidth with which 95% of d-aps can be expressed
  int lowexcp;   // 
  int lowest;    // bitwidth of smalled d-gap
  int highest;   // bitwidth of largest d-gap

  double modFrac;   // fraction of d-gaps with modal bitwidth
  double lowFrac;   // fraction of d-gaps with smaller bitwidth than mode
  double highFrac;  // fraction of d-gaps with larger bitwidth than mode

  int totalrange;  // from highest to lowest bitwidths
  int range;       // from high exception to low exception bitwidths

  struct record
	  {
	  uint lst;  // lowest bitwidth
	  uint hst;  // hghest bitwidth
	  uint hxp;  // high exception bitwidth 
	  uint md;   // modal bitwidth
	  };
  
  ListStats(int listID, int length)
	  {
	  listNumber = listID;
	  listLength = length;
	  }

  /*
	  convert an array of document numbers to an array of d-gaps
  */
  void docnums_to_dgaps(uint32_t *dest, int *source, int length);
  
  /*
	  convert an array of d-gaps to an array of number of bits required to
	  express those d-gaps
  */ 
  //void dgaps_to_bitwidths(int *dest, int *source, int length);

  /* 
	  convert an array of document numbers to an array of bitwidths of d-gaps
  */
  void docnums_to_dgap_bitwidths(int *dest, int *source, int length);

  /* 
	  efficiently calculate the number of bits needed for the binary
	  representation. Copied from here:
	  https://github.com/torvalds/linux/blob/master/include/asm-generic/bitops/fls.h
  */
  int getBitwidth(uint x); 

  /* 
	  print a short version of the list statistics
  */
  void print_stats_short(void);

  /* 
	  print a long version of the list statistics
  */
  void print_stats(void);

  /* 
	  encode the lists statistics in to 32 bits
  */
  void encode_stats(uint *dest);
  
  /* 
	  decode the list statistics from the 32-bits to a ListStats::record structure
  */
  record decode_stats(uint *encoded);
  
  /* 
	  print the list statistics from the record struct
  */
  void print_stats_record(record stats);
  
  /* 
	  Calculate statistics of a list for use in selector generator
	  Lists should already be of bitwidths of dgaps when passed to this function
  */
  void calculate_stats(const int *bitwidths, int length);

};
