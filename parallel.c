#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define LENGTH 100
/* keep length under 256 for now so we can use 8 bits for packing
 and treat selector chosing as a separate problem */


/* print an unsigned 32 bit int in big-endian binary */
void print_bigendian(uint32_t num)
{
    int i;
    for (i = 31; i >= 0; i--) {
        if (num & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}


/* print an unsigned 32 bit int in little-endian binary */
void print_littleendian(uint32_t num)
{
    int i;
    for (i = 0; i < 32; i++) {
        if (num & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}



int main(void)
{
    int i;
    uint32_t *dgaps, decompressed;
    uint64_t *compressed;
    uint8_t *selectors;

    dgaps = malloc(LENGTH * sizeof(*dgaps));
    for (i = 0; i < LENGTH; i++) {
        dgaps[i] = i;
    }

    for ( i = 0; i < LENGTH; i++) {
        //
        +printf("%d%c", dgaps[i], i == LENGTH - 1 ? '\n' : ' ');
    }
    
    for ( i = 0; i < LENGTH; i++) {
        //print_bigendian(dgaps[i]);
        //print_littleendian(dgaps[i]);
    }
    
}

