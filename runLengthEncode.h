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
	int encode(uint8_t *dest, int *source, int length);
	int decode(int *decompressed, uint8_t *encoded, int length);

	};
