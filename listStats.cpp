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
int lowest;
int highest;
int range;
int totalrange;

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
  // first docnum may be zero, in which case fls won't give correct bitwidth
  if (source[0] == 0)
    dest[0] = 1;
  else
    dest[0] = fls(source[0]);
  
  for (int i = 1; i < length; i++)
    dest[i] = fls(source[i]);
}


void ListStats::docnums_to_dgap_bitwidths(int *dest, int *source, int length)
{
  // first docnum may be zero, in which case fls won't give correct bitwidth
  // i should write my own bitwidth function...
  if (source[0] == 0)
    dest[0] = 1;
  else
    dest[0] = fls(source[0]);
  
  int prev = source[0];
  for (int i = 1; i < length; i++)
  {
    int temp = source[i] - prev;
    prev = source[i];
    dest[i] = fls(temp);
  }
}
 
/* 
   Calculate statistics of a list for use in selector generator
   Lists should already be of dgaps when passed to this function
*/
void ListStats::calculate_stats(const int *bitwidths, int length)
{
  double sum = 0;
  double fraction = 0;
  int max = 0;
  bool set95th = false, set90th = false;
  int highoutliers = 0;
  int lowoutliers = 0;
  int ninetyfifth = 0;
  int ninetieth = 0;
  
  /*
    Count frequencies of bitwidths and calculate mean and stddev
   */
  int *width_freqs = new int[33];
  for (int i = 0; i < 33; i++)
    width_freqs[i] = 0;

  stdev = 0;
  for (int i = 0; i < length; i++)
  {
    width_freqs[bitwidths[i]]++;
    sum += bitwidths[i];
  }
  mean = sum / length;

  for (int i = 0; i < length; i++)
    stdev += pow(bitwidths[i] - mean, 2);
  stdev = sqrt(stdev/length);


  

  /*
    Find modal bitwidth and high exception
   */
  sum = 0;
  for (int i = 0; i < 32; i++)
  {
    sum += width_freqs[i];
    fraction = sum / length;
    if (width_freqs[i] >= max)
    {
      max = width_freqs[i];
      mode = i;
    }
    if (set95th == false && fraction >= 0.95)
    {
      set95th = true;
      ninetyfifth = i;
    }
    if (set90th == false && fraction >= 0.90)
    {
      set90th = true;
      ninetieth = i;
    }
  }
  highexcp = ninetyfifth;
  
  for (int i = ninetyfifth; i < 32; i++)
    if (width_freqs[i])
      highest = i;
  /* 
     having looked at stats of short and long lists, i think it will
     make most sense to use 95th percentile for high exceptions. but
     will be worth collecting imperical data on this.  other
     possibilities include 90th percentile and largest value.
  */

  
  /* 
     find exception frequencies 
  */
  for (int i = 0; i < mode; i++)
    lowoutliers += width_freqs[i];

  for (int i = mode + 1; i < 32; i++)
    highoutliers += width_freqs[i];

  
  modFrac = (double) width_freqs[mode] / length;
  lowFrac = (double) lowoutliers / length;
  highFrac = (double) highoutliers / length;


  //printf("\nfraction of high outliers: %.2f\n", (double) highoutliers / length);
  //printf("fraction of low outliers: %.2f\n", (double) lowoutliers / length);
    
  /* 
     find next most frequent bitwidth smaller than the mode
  */
  max = 0;
  for (int i = 0; i < mode; i++) 
    if (width_freqs[i] >= max)
    {
      max = width_freqs[i];
      lowexcp = i;
    }
  if (width_freqs[lowexcp] == 0)
    lowexcp = mode;

  /* 
     find smallest bitwidth
   */
  for (int i = 0; i < 32; i++)
    if (width_freqs[i])
    {
      lowest = i;
      break;
    }

  totalrange = 1 + highest - lowest;
  range = 1 + highexcp - lowexcp;


 


  
  
  /*
    Leave this here, will show up in regression tests if this ever
    happens (shouldn't be possible with wsj collection of course,
    largest docno fits in 18 bits)
  */
  for (int i = 19; i < 33; i++)
    if (width_freqs[i])
      printf("found a bitwidth greater than 18, this could be a problem\n");

  delete [] width_freqs;
}


