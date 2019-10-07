#pragma once
#include <stdint.h>

class RunLengthEncode
	{
	public:

	RunLengthEncode()
		{
		
		}

	~RunLengthEncode()
		{

		}

	public:
	int runlength_encode(uint8_t *dest, int *source, int length);

	};
