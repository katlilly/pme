#pragma once

#include <cstdint>
#include "selectorGen.h"

class CompressPME
	{
	public:
	int selector_bits;
	
	CompressPME()
		{
		// nothing
		}
	
	~CompressPME()
		{
		// nothing
		}

	public:
	int encode(uint32_t *dest, uint32_t *raw, int n_dgaps_to_compress);
	//int encode(uint32_t *dest, uint32_t *raw, SelectorGen::selector_table *table, int n_dgaps_to_compress);
	
	int decode(uint32_t *dest, uint32_t *compressed, int n_dgaps_to_decompress);

	};
