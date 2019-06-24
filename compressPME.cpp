#include <stdio.h>
#include <cstdint>
#include "compressPME.h"

void CompressPME::print_selector_table(void)
{
  printf("Selector bits: %d (max selectors: %d)\n", selector_bits, max_selectors);
  printf("Number of selectors in this table: %d\n", st.rows);
  for (int i = 0; i < st.rows; i++)
  {
    printf("%d: ", st.ints_to_pack[i]);
    for (int j = 0; j < st.ints_to_pack[i]; j++)
      printf("%2d ", st.bit_widths[i][j]);
    printf("\n");
  }
  printf("\n");
}

void CompressPME::print_bigendian32(uint32_t num)
{
  int i;
  for (i = 31; i >= 0; i--) 
    if (num & (1<<i))
      printf("1");
    else 
      printf("0");

  printf("\n");
}


// uint32_t CompressPME::pme_decompress(uint32_t word, uint32_t selector, int offset)
// {
//     int i, bits, intsout = 0;
//     uint32_t mask, payload;
//     payload = word;
//     for (i = 0; i < table[selector].intstopack; i++) {
//         bits = table[selector].bitwidths[i];
//         mask = pow(2, bits) - 1;
//         decoded[intsout + offset] = payload & mask;
//         intsout++;
//         payload = payload >> bits;
//     }
//     return intsout;
// }


/* this one called in a loop until done*/
// uint32_t CompressPME::pme_encode(uint32_t *destination, uint32_t *raw, uint8_t *selectors,
//                     uint32_t intstocompress)
// {
//     int i;
//     uint32_t which;                             /* which row in selector array */
//     int column;                                 /* which element in bitwidth array */
//     int current;                                /* count of elements within each compressed word */
//     int topack;                                 /* min of ints/selector and remaining data to compress */
//     uint32_t *integer = raw;                    /* the current integer to compress */
//     uint32_t *end = raw + intstocompress;       /* the end of the input array */
    
//     /* choose selector */
//     uint32_t *start = integer;
//     for (which = 0; which < rownumber; which++) {
//         column = 0; /* go back to start of each row because of way some selectors may be ordered */
//         integer = start; /* and also go back to first int that needs compressing */
//         topack = min(intstocompress, table[which].intstopack);
//         end = raw + topack;
//         for (; integer < end; integer++) {
//             if (fls(*integer) > table[which].bitwidths[column]) {
//                 break; /* increment 'which' if current integer can't fit in this many bits */
//             }
//             column++;
//         }
//         if (integer >= end) {
//             break;
//         }
//     }
    
//     /* pack one word */
//     *destination = 0;
//     //*selectors = 0;
//     *selectors = 0 | which;
//     i = 0;
//     int shiftdistance = 0;
//     for (current = 0; current < topack; current++) {
//         *destination = *destination | raw[current] << shiftdistance;
//         shiftdistance += table[which].bitwidths[i];
//         i++;
//     }
//     return topack;   /* return number of dgaps compressed into this word */
// }
