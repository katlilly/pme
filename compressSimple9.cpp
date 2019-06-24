#include "compressSimple9.h"
#include "fls.h"


/* 
   return minimum, or first param if same
*/
uint32_t CompressSimple9::min(uint32_t a, uint32_t b)
{
    return a <= b ? a : b;
}


uint32_t CompressSimple9::encode(uint32_t *dest, uint32_t *raw, uint32_t intstocompress)
{
  uint32_t which;                             /* which element in selector array */
  int current;                                /* count of elements within each compressed word */
  int topack;                                 /* min of intstopack and what remains to compress */
  uint32_t *integer = raw;                    /* the current integer to compress */
  uint32_t *end = raw + intstocompress;       /* the end of the input array */
  uint32_t code, shiftedcode;
    
  /* chose selector table row */
  for (which = 0; which < 9; which++)
    {
      topack = min(intstocompress, table[which].intstopack);
      end = raw + topack;
      for (; integer < end; integer++)
      	if (fls(*integer) > table[which].bits)
	  break;
      
      if (integer >= end)
	break;
    }
    
  /* pack one word */
  *dest = *dest | which;
  for (current = 0; current < topack; current++) {
    code = raw[current];
    shiftedcode = code << (4 + (current * table[which].bits));
    *dest = *dest | shiftedcode;
  }
  return topack;    /* return number of dgaps compressed into this word */
}


uint32_t CompressSimple9::decode(uint32_t *decoded, uint32_t word, int offset)
{
  uint32_t i, intsout = 0;
  uint32_t selector, payload, temp;
  selector = word & 0xf;
  //uint32_t mask = table[selector].masks;
  payload = word >> 4;
  for (i = 0; i < table[selector].intstopack; i++) {
    temp = payload & table[selector].masks;
    decoded[intsout + offset] = temp;
    intsout++;
    payload = payload >> table[selector].bits;
  }
  return intsout;
}
