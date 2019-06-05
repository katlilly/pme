#include <stdio.h>
#include <stdlib.h>
#include "listStats.h"
#include "fls.h"
#include <math.h>

// ?? where to calculate this?
int numperms;

// set by constructor
int listNumber;
int listLength;

// calculated in calculate_stats()
double mean;
double stdev;
int mode;
int lowexcp;
int highexcp;
double modFrac;
double lowFrac;
double highFrac;


void ListStats::docnums_to_dgaps(int *dest, int *source, int length)
{
  int prev = 0;
  for (int i = 0; i < length; i++)
    {
      dest[i] = source[i] - prev;
      prev = source[i];
    }
}

/*
  DocIDs are zero indexed in "postings.bin", 
 */
void ListStats::dgaps_to_bitwidths(int *dest, int *source, int length)
{
  // first docnum may be zero, - in which case fls won't give correct bitwidth
  if (source[0] == 0)
    dest[0] = 1;
  else
    dest[0] = fls(source[0]);
  
  for (int i = 1; i < length; i++)
    dest[i] = fls(source[i]);

}


void ListStats::docnums_to_dgap_bitwidths(int *dest, int *source, int length)
{
  // first docnum may be zero, - in which case fls won't give correct bitwidth
  if (source[0] == 0)
    dest[0] = 1;
  else
    dest[0] = fls(source[0]);
  
  int prev = source[0];
  for (int i = 1; i < length; i++)
  {
    dest[i] = source[i] - prev;
    prev = source[i];
    dest[i] = fls(dest[i]);
  }
}
 
/* 
   Calculate statistics of a list for use in selector generator
   Lists should already be of dgaps when passed to this function
*/
void ListStats::calculate_stats(int *bitwidths, int length)
{
  /*
    Count frequencies of bitwidths and calculate mean and stddev
   */
  int *width_freqs = new int[33];
  for (int i = 0; i < 33; i++)
    width_freqs[i] = 0;

  double sum = 0;
  double mean;
  double stdev = 0;
  for (int i = 0; i < length; i++)
  {
    width_freqs[bitwidths[i]]++;
    sum += bitwidths[i];
  }
  mean = sum / length;
  for (int i = 0; i < length; i++)
  {
    stdev += pow(bitwidths[i] - mean, 2);
  }
  stdev = sqrt(stdev/length);
  
  printf("mean bitwidth of list %d: %.0f +/- %.2f\n", listNumber, mean, stdev);
  
  
  
  if (width_freqs[32])
    printf("this could be a problem\n");
}
