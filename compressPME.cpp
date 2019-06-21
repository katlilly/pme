#include <stdio.h>
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
