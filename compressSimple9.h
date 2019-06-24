#pragma once

#include <cstdint>

class CompressSimple9
{
 public:

  //typedef enum {bits, intstopack, masks} s;
    
  struct selector
  {
    uint32_t bits;
    uint32_t intstopack;
    uint32_t masks;
  };

  selector *table;

  
  
 public:
  CompressSimple9()
    {
      table = new selector[9];
      table[0] = {1, 28, 1};
      table[1] = {2, 14, 3};
      table[2] = {3,  9,  7};
      table[3] = {4,  7,  0xf};
      table[4] = {5,  5,  0x1f};
      table[5] = {7,  4,  0x7f};
      table[6] = {9,  3,  0x1ff};
      table[7] = {14, 2,  0x3fff};
      table[8] = {28, 1,  0xfffffff};
    }


  
  ~CompressSimple9()
    {
      for (int i = 0; i < 9; i++)
	delete &table[i];
      delete [] table;
    }

 public:
  uint32_t encode(uint32_t *dest, uint32_t *raw, uint32_t intstocompress);
  uint32_t decode(uint32_t *decoded, uint32_t word, int offset);

 private:
  uint32_t min(uint32_t a, uint32_t b);


};
