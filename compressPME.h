#pragma once

#include <cstdint>
#include "selectorGen.h"

class CompressPME
	{
	public:
	int selector_bits;

	struct record
		{
		int n_dgaps_compressed;  // number of dgaps compressed
		int compressed_size;     // number of bytes in compressed data
		};
	
	CompressPME()
		{
		// nothing
		}
	
	~CompressPME()
		{
		// nothing
		}

	public:
	//int encode(uint32_t *dest, int *raw, int n_dgaps_to_compress);
	CompressPME::record encode(uint32_t *dest, uint32_t *raw, SelectorGen::selector_table *table, int n_to_compress);
	int encode_one_word(uint32_t *dest, uint32_t *raw, SelectorGen::selector_table *table, int n_dgaps_to_compress);
	int decode(uint32_t *dest, uint32_t *compressed, int n_dgaps_to_decompress);

	private:
	int min(uint a, uint b);
	int get_bitwidth(uint x); 
	
	
	};
