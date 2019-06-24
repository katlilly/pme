#pragma once

#include <cstdint>

class CompressPME
{
 public:
  int selector_bits;

 private:
  int max_selectors;
  int payload_bits;
  
  struct table {
    int rows;
    int *ints_to_pack;
    int **bit_widths;
  };
  table st;
  
 public:
  CompressPME(int selbits, int **table)
    {
      selector_bits = selbits;
      payload_bits = 32 - selbits;
      
      max_selectors = 2;
      for (int i= 1; i < selbits; i++)
	max_selectors *= 2;
      
      st.rows = max_selectors;
      for (int i = 0; i < max_selectors; i++)
	if (table[i][0] == 0)
	{
	  st.rows = i;
	  break;
	}
      st.ints_to_pack = new int[st.rows];
      st.bit_widths = new int*[st.rows];

      for (int i = 0; i < st.rows; i++)
      {
	int rowlength = payload_bits;
	for (int j = 0; j < payload_bits; j++)
	{
	  if (table[i][j] == 0)
	  {
	    rowlength = j;
	    break;
	  }
	  st.bit_widths[i] = new int[rowlength];
	}
	for (int j = 0; j < rowlength; j++)
	  st.bit_widths[i][j] = table[i][j];
	st.ints_to_pack[i] = rowlength;
      }

      // need to sort the table so longer selectors never occur after
      // shorter ones, otherwise the best selector won't always get chosen
      
    }

  ~CompressPME()
    {
      for (int i = 0; i < st.rows; i++)
      {
	delete [] st.bit_widths[i];
      }
      delete [] st.bit_widths;
      delete [] st.ints_to_pack;
    }
      

 public:
  void print_selector_table();
  void print_bigendian32(uint32_t num);
  uint32_t pme_encode(uint32_t *dest, uint32_t *raw, uint8_t *selectors,
		      uint32_t intstocompress);

};
