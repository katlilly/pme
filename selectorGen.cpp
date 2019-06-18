#include <stdio.h>
#include <stdlib.h>
#include "selectorGen.h"

void SelectorGen::generate(int **dest)
{
  

}

void SelectorGen::print_table(int **table)
{
  for (int i = 0; i < num_selectors; i++)
    {
      for (int j = 0; j < 28; j++)
	printf("%d ", table[i][j]);
    }
  printf("\n");
}
