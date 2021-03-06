#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <cstdint>
#include "listStats.h"

/*
  convert an array of document numbers to an array of d-gaps
 */
void ListStats::docnums_to_dgaps(uint32_t *dest, int *source, int length)
	{
	int prev = 0;
	for (int i = 0; i < length; i++)
		{
		dest[i] = source[i] - prev;
		prev = source[i];
		}
	}

/*
  convert an array of d-gaps to an array of number of bits required to
  express those d-gaps
*/
 void ListStats::dgaps_to_bitwidths(uint32_t *dest, uint32_t *source, int length)
 	{
 	for (int i = 0; i < length; i++)
 		dest[i] = getBitwidth(source[i]);
 	}

/* 
	convert an array of document numbers to an array of bitwidths of d-gaps
*/
void ListStats::docnums_to_dgap_bitwidths(int *dest, int *source, int length)
	{
	dest[0] = getBitwidth(source[0]);
	int prev = source[0];

	for (int i = 1; i < length; i++)
		{
		int temp = source[i] - prev;
		prev = source[i];
		dest[i] = getBitwidth(temp);
		}
	}

/* 
	efficiently calculate the number of bits needed for the binary
	representation. Copied from here:
	https://github.com/torvalds/linux/blob/master/include/asm-generic/bitops/fls.h
 */
int ListStats::getBitwidth(uint x)
	{
	int r = 32;

	if (!x)
		return 1;
	if (!(x & 0xffff0000u))
		{
		x <<= 16;
		r -= 16;
		}
	if (!(x & 0xff000000u))
		{
		x <<= 8;
		r -= 8;
		}
	if (!(x & 0xf0000000u))
		{
		x <<= 4;
		r -= 4;
		}
	if (!(x & 0xc0000000u))
		{
		x <<= 2;
		r -= 2;
		}
	if (!(x & 0x80000000u))
		{
		x <<= 1;
		r -= 1;
		}
	return r;
	}

/* 
	print a short version of the list statistics
*/
void ListStats::print_stats_short(void)
	{
	printf("%d, %d, %d, %d\n", lowexcp, mode, highexcp, highest);
	}

/* 
	print a long version of the list statistics
*/
void ListStats::print_stats(void)
	{
	printf("List number %d, length: %d\n", listNumber, listLength); 
	printf("Mean bitwidth = %.2f +/- %.2f\n", mean, stdev); 
	printf("Modal bitwidth %d\n", mode); 
	printf("High and low exceptions chosen: %d, %d\n", highexcp, lowexcp); 
	printf("Highest and lowest bitwidths: %d, %d\n", highest, lowest);
	printf("Low, modal, high fractions: %.2f, %.2f, %.2f\n", lowFrac, modFrac, highFrac);
	printf("Chosen range and total range: %d, %d\n\n", range, totalrange); 
	}

/* 
	encode the lists statistics in to 32 bits
*/
void ListStats::encode_stats(uint32_t *dest)
	{
	*dest = 0;
	*dest |= (mode << 24);
	*dest |= (highexcp << 16);
	*dest |= (highest << 8);
	*dest |= lowexcp;
	}

/* 
	decode the list statistics from the 32-bits to a ListStats::record structure
*/
ListStats::record ListStats::decode_stats(uint *encoded)
	{
	ListStats::record result;
	uint code = *encoded;
	result.lst = code & 0xff;
	code = code >> 8;
	result.hst = code & 0xff;
	code = code >> 8;
	result.hxp = code & 0xff;
	code = code >> 8;
	result.md = code & 0xff;
	return result;
	}

/* 
	print the list statistics from the record struct
*/
void ListStats::print_stats_record(ListStats::record stats)
	{
	printf("Lowest: %d\nMode: %d\nHighexp: %d\nHighest: %d\n\n", stats.lst, stats.md, stats.hxp, stats.hst);
	}

/* 
   Calculate statistics of a list for use in selector generator
   Lists should already be of bitwidths of dgaps when passed to this function
*/
void ListStats::calculate_stats(const int *bitwidths, int length)
	{
	double sum = 0;
	int max = 0;
	bool set95th = false;
	int highoutliers = 0;
	int lowoutliers = 0;
	int ninetyfifth = 0;
  
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
		double fraction = sum / length;
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
		}
	highexcp = ninetyfifth;
  
	for (int i = ninetyfifth; i < 32; i++)
		if (width_freqs[i])
			highest = i;

	/* having looked at stats of short and long lists, i think it will
		make most sense to use 95th percentile for high exceptions. but
		will be worth collecting imperical data on this.  other
		possibilities include 90th percentile and largest value. */

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
	  shouldn't be possible with wsj collection, largest docno fits in
	  18 bits
	*/
	for (int i = 19; i < 33; i++)
		if (width_freqs[i])
			printf("found a bitwidth greater than 18, this could be a problem\n");

	delete [] width_freqs;
	}
