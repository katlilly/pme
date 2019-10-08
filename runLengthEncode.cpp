#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "runLengthEncode.h"

int RunLengthEncode::encode(uint8_t *dest, int *source, int length)
	{
	//printf("%d columns in this selector\n", length);

	int count = 0;
	int runlength = 0;
	for (int index = 0; index < length; )
		{
		int current = source[index];
		if (current == source[index + 1])
			{
			// keep looking fro end of run
			runlength++;
			index++;
			}
		else
			{
			// have found the end of a run, now write it out
			index++;
			while(runlength > 7)
				{
				dest[count] = 0;
				dest[count] |= 7;
				dest[count] |= current << 3;
				runlength -= 8;
				count++;
				}
			dest[count] = 0;
			dest[count] |= runlength;
			dest[count] |= current << 3;
			count++;
			current = source[index];
			runlength = 0;
			}
		}
	return count; // return number of bytes written
	}


int RunLengthEncode::decode(int *decompressed, uint8_t *encoded, int length)
	{
	int count = 0;
	for (int i = 0; i < length; i++)
		{
		int value = (encoded[i] & 248) >> 3;
		int repeats = encoded[i] & 7;
		decompressed[count++] = value;
		for (int j = 0; j < repeats; j++)
			decompressed[count++] = value;
		}
	return count;  // return number of column widths decompressed
	}
