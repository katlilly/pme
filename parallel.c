#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>


#define LENGTH 100
/* keep length under 256 for now so we can use 8 bits for packing
 and treat selector chosing as a separate problem for now */


/* print an unsigned 64 bit int in big-endian binary */
void print_bigendian64(uint32_t left, uint32_t right)
{
    int i;
    for (i = 31; i >= 0; i--) {
        if (left & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    for (i = 31; i >= 0; i--) {
        if (right & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}


/* print an unsigned "64 bit int" in little-endian binary */
void print_littleendian64(uint32_t left, uint32_t right)
{
    int i;
    for (i = 0; i < 32; i++) {
        if (left & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    for (i = 0; i < 32; i++) {
        if (right & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}

/* print an unsigned "128 bit int" in big-endian binary */
void print_bigendian128(uint32_t one, uint32_t two, uint32_t three, uint32_t four)
{
    int i;
    for (i = 31; i >= 0; i--) {
        if (i % 8 == 7) {
            printf(" ");
        }
        if (one & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf(" ");
    for (i = 31; i >= 0; i--) {
        if (i % 8 == 7) {
            printf(" ");
        }
        if (two & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf(" ");
    for (i = 31; i >= 0; i--) {
        if (i % 8 == 7) {
            printf(" ");
        }
        if (three & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf(" ");
    for (i = 31; i >= 0; i--) {
        if (i % 8 == 7) {
            printf(" ");
        }
        if (four & (1<<i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}




/* print an unsigned 32 bit int in big-endian binary */
void print_bigendian32(uint32_t num)
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
void print_littleendian32(uint32_t num)
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
    uint32_t *dgaps, *compressed, *decompressed;
    uint8_t *selectors;

    /* write arbitrary data to dgaps array */
    dgaps = malloc(LENGTH * sizeof(*dgaps));
    for (i = 0; i < LENGTH; i++) {
        dgaps[i] = i + 1;
    }
    for (i = 0; i < LENGTH; i++) {
        //printf("%d%c", dgaps[i], i == LENGTH - 1 ? '\n' : ' ');
    }
    
    
    compressed = malloc(LENGTH * sizeof(*compressed));
    decompressed = malloc(LENGTH * sizeof(*decompressed));
    selectors = malloc(LENGTH * sizeof(*selectors));
    
    /* this doesn't seem necessary, but why not? */
    for (i = 0; i < LENGTH; i++) {
        compressed[i] = 0;
    }
    
    
    
    /* compress into imaginary uint128_t array (using uint32_t array) */
    int intscompressed = 0; /* position in dgaps array */
    int cindex = 0; /* position in compressed array */
    int shiftdist = 0; /* which byte in word32 */
    while (intscompressed < LENGTH) {
        compressed[cindex] |= (dgaps[intscompressed] << shiftdist);
        cindex++;
        intscompressed++;
        if (cindex % 4 == 0 && shiftdist == 24) {
            shiftdist = 0;
        } else if (cindex % 4 == 0) {
            shiftdist += 8; /* change literal to bitwidth variable later? */
            cindex -= 4;
        }
    }
    
    
    //(compressed[0], compressed[1], compressed[2], compressed[3]);
    //print_bigendian128(compressed[4], compressed[5], compressed[6], compressed[7]);
    
    /* decompress to screen, in 4byte word order (1, 5, 9, 13, 2, 6, 10, etc) */
    int j = 0;
    for (i = 0; i < cindex + 4; i++) {
        printf("%2d, ", (compressed[i] & 0xff));
        printf("%2d, ", (compressed[i] & 0xff00) >> 8);
        printf("%2d, ", (compressed[i] & 0xff0000) >> 16);
        printf("%2d,  ", (compressed[i] & 0xff000000) >> 24);
        if (i % 4 == 3) printf("\n");
    }
    /* need to keep track of ints decompressed ? or while decompressed int |= zero? */
   
    
    /* decompress */
    int intsdecompressed = 0;
    int mask = 0xff;
    cindex = 0;
    shiftdist = 0;
    while (intsdecompressed < LENGTH) {
        decompressed[intsdecompressed++] = ((compressed[cindex++] << shiftdist) & mask);
        if (cindex % 4 == 0) {
            cindex -= 4;
            shiftdist += 8;
//            mask = mask >> 8;
        }
    }
    
    for (i = 0; i < 20; i++) {
        printf("%d\n", decompressed[i]);
    }

    free(dgaps);
    free(compressed);
    free(decompressed);
    free(selectors);
    
    return 0;
    
}

