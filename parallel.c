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

    
    /* for now just using symmetric selectors, value in this array
     represents the bitwidth */
//    for (i = 0; i < LENGTH; i++) {
//        selectors[i] = 8;
//    }
 
    
//    int bitwidth = 8;
//    int ints_per_word = sizeof(*compressed) / sizeof(uint8_t);
//
//    int intscompressed;
//    int dgapsindex = 0;
//    int compressedwords = 0;
//    int shiftdist = 0;
//    while (dgapsindex < LENGTH) {
//        compressed[compressedwords] = 0;
//        intscompressed = 0;
//        while (intscompressed < ints_per_word) {
//            compressed[compressedwords] |= (dgaps[dgapsindex] << 8 * shiftdist);
//            //print_bigendian64(compressed[compressedwords]);
//            dgapsindex++;
//            intscompressed++;
//            shiftdist++;
//        }
//        compressedwords++;
//    }
//
//    uint32_t zero = 0;
//    uint32_t two55 = 255;
//    uint32_t ffffff = 0xffffff;
//    print_littleendian128(zero, two55, two55, ffffff);

    /* write selectors and compress dgaps */
    int compressedwords = 0;
    int selector = 8; /* for now this means bitwidth */
    
    int dgaps_per_128 = 128 / 8;
    int compresseddgaps; /* compressed ints in one 128 bit word (index within a compressed word) */
    int compressedints = 0; /* total compressed ints (dgaps index) */
    int shiftdist = 0;
    int subword;
    while (compressedints < LENGTH) {
        selectors[compressedwords] = selector;
        subword = 0;
        shiftdist = 0;
        compresseddgaps = 0;
        while (compresseddgaps < dgaps_per_128 && compressedints < LENGTH) {
            /* below line is wrong, need to thing through position of numbers in 2,3, 4, subwords */
            compressed[compressedwords + subword++] |= (dgaps[compressedints++] << (8 * shiftdist + subword * 4));
            if (subword == 4) {
                subword = 0;
                shiftdist++;
            }
            compresseddgaps++;
        }
        compressedwords++;
    }
    
    print_bigendian128(compressed[0], compressed[1], compressed[2], compressed[3]);
    print_bigendian128(compressed[4], compressed[5], compressed[6], compressed[7]);
    
    /* decompress to screen, in 4byte word order (1, 5, 9, 13, 2, 6, 10, etc) */
    for (i = 0; i < compressedwords; i++) {
        printf("%d, ", compressed[i] & 0xff);
        printf("%d, ", (compressed[i] & 0xff00) >> 8);
        printf("%d, ", (compressed[i] & 0xff0000) >> 16);
        printf("%d, ", (compressed[i] & 0xff000000) >> 24);
        printf("\n");
    }
    
    /* decompress */
    

    free(dgaps);
    free(compressed);
    free(decompressed);
    free(selectors);
    
    return 0;
    
}

