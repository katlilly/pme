#include <stdint.h>
#include <stdio.h>
#include "runLengthEncode.h"

int RunLengthEncode::runlength_encode(uint8_t *dest, int *source, int length)
	{
	printf("%d columns in this selector\n", length);
	
	int runlength = 0;
	for (int index = 0; index < length; )
		{
		int current = source[index];
		if (current == source[index + 1])
			{
			runlength++;
			index++;
			}
		else
			{
			index++;
			printf("value: %d, runlength: %d\n", current, runlength);
			current = source[index];
			runlength = 0;
			}
		
		}
	
	

	return length;
	}
