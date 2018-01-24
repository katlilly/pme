#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#define NUMBER_OF_DOCS (1024 * 1024 * 128)

static uint32_t *postings_list;

int numperms; /* number of distinct permutations of a bitwidth combination */
int *comb; /* bitwidth combination array generated for each list */
int topack; /* number of ints in generated combination */
int rowsfilled; /* keep track of row number when generating selector table */
int plainrows; /* how many of the symmetric selectors have been used */

/* data structure for statistical data for a single list */
typedef struct {
    int listNumber;
    int listLength;
    double mean;
    double stdev;
    int mode;
    int lowexcp;
    int highexcp;
    double modFrac;
    double lowFrac;
    double highFrac;
    int numPerms;
} listStats;


/* data structure for each row of selector table
   using 8 bit selectors so will declare a 256 row table */
typedef struct {
    int intstopack;
    int *bitwidths;
} selector;

selector table[256];


int compare_ints(const void *a, const void *b) {
    const int *ia = (const int *) a;
    const int *ib = (const int *) b;
    return *ia < *ib ? -1 : *ia == *ib ? 0 : 1;
}


/* print a permutation to screen */
void output_perms(int *array, int length)
{
    int i;
    for (i = 0; i < length; i++) {
        printf("%d%c", array[i], i == length - 1 ? '\n' : ' ');
    }
}

/* populate selector table which has been declared as global data */
void add_perm_to_table(int *array, int length)
{
    table[rowsfilled].intstopack = length;
    table[rowsfilled].bitwidths = malloc(length * sizeof(*table[rowsfilled].bitwidths));
    for (int i = 0; i < length; i++) {
        table[rowsfilled].bitwidths[i] = array[i];
    }
    
}


/* get next lexicographical permutation. taken from rosettacode */
int next_lex_perm(int *a, int n) {
#define swap(i, j) {t = a[i]; a[i] = a[j]; a[j] = t;}
    int k, l, t;
    
    /* 1. Find the largest index k such that a[k] < a[k + 1]. If no such
     index exists, the permutation is the last permutation. */
    for (k = n - 1; k && a[k - 1] >= a[k]; k--);
    
    if (!k--) return 0;
    /* 2. Find the largest index l such that a[k] < a[l]. Since k + 1 is
     such an index, l is well defined */
    for (l = n - 1; a[l] <= a[k]; l--) {
        ;
    }
    
    /* 3. Swap a[k] with a[l] */
    swap(k, l);
    
    /* 4. Reverse the sequence from a[k + 1] to the end */
    for (k++, l = n - 1; l > k; l--, k++)
        swap(k, l);
    return 1;
#undef swap
}


/* generates permutations in correct order and outputs unique ones
 - taken directly from rosettacode */
void generate_perms(int *x, int n, void callback(int *, int))
{
    do {
        if (callback) callback(x, n);
        numperms++;
        rowsfilled++;
    } while (next_lex_perm(x, n) && rowsfilled - plainrows < 246);
    /* second condition leaves room for 10 symmetric selectors */
}


/* make combination selector for a list with given statistics */
int * make_combs(int mode, double modeFrac, int low, double lowFrac,
                 int high, double highFrac)
{
    int bitsused, numInts, tries, i, ext_wasted_bits;
    
    int payload = 32;
    int numLow = 1, numMode = 1, numHigh = 1;
    double lowusedfrac, modeusedfrac, highusedfrac;
    
    if (low == 0) {
        low = 1;
    }
    
    /* count correct number of ints of each bitwidth to use */
    if (low + high + mode <= payload) {
        numInts = numLow + numMode + numHigh;
        bitsused = low + mode + high;
        lowusedfrac = (double) numLow / numInts;
        modeusedfrac = (double) numMode / numInts;
        highusedfrac = (double) numHigh / numInts;
        tries = 0;
        while (bitsused < payload) {
            tries++;
            if (highusedfrac < highFrac && bitsused + high <= payload) {
                numHigh++;
                bitsused += high;
                numInts++;
                highusedfrac = (double) numHigh / numInts;
            }
            if (modeusedfrac < modeFrac && bitsused + mode <= payload) {
                numMode++;
                bitsused += mode;
                numInts++;
                modeusedfrac = (double) numMode / numInts;
            }
            if (lowusedfrac < lowFrac && bitsused + low <= payload) {
                numLow++;
                bitsused += low;
                numInts++;
                lowusedfrac = (double) numLow / numInts;
            }
            /* most cases break out of loop here, some get stuck bc frac */
            if (bitsused + low > payload
                && bitsused + high - mode > payload
                && bitsused + mode - low > payload) {
                break;
            }
            /* add extra ints to fill space regardless of modfrac, catches a few
             cases that can't break out because of exception frequencies */
            /**** 20 is a made up number and is prob bigger than needed  ****/
            if (tries > 20) {
                /* best option is to promote one mode to high exception */
            /**** order of if-elses is a judgement call - revisit this ****/
                if (bitsused + high - mode <= payload
                    && numMode > 0) {
                    numMode--;
                    numHigh++;
                    bitsused += high - mode;
                }
                /* next best is to add a mode */
                if (bitsused + mode <= payload) {
                    numMode++;
                    bitsused += mode;
                }
                /* else try promote a low to a mode */
                if (bitsused + mode - low <= payload) {
                    numLow--;
                    numMode++;
                    bitsused += mode - low;
                }
                /* last option is to add a low exception */
                if (bitsused + low <= payload) {
                    numLow++;
                    bitsused += low;
                } else {
                    break;
                }
            }
        }
        
        /* check for errors: pipe output to grep wasted or grep overflow */
        ext_wasted_bits = payload - numLow*low - numMode*mode - numHigh*high;
        if (ext_wasted_bits > low) {
            printf("%d wasted bits\n", ext_wasted_bits);
        }
        if (bitsused > 32) {
            printf("more than 32 bits used\n");
        }
        
        /* fill combination array */
        numInts = numLow + numHigh + numMode;
        comb = malloc(numInts * sizeof(*comb));
        for (i = 0; i < numLow; i++) {
            comb[i] = low;
        }
        for (i = numLow; i < numLow + numMode; i++) {
            comb[i] = mode;
        }
        for (i = numLow + numMode; i < numInts; i++) {
            comb[i] = high;
        }
        
        topack = numInts; /* send number of ints in comb to global var */
        qsort(comb, topack, sizeof(*comb), compare_ints);
        return comb;
        
    } else {
        /* deal with case where high + low + mode > 32 */
        /* currently just have two options, pack 1 or 2 ints */
        /**** add extra conditions for assymetric cases ****/
        if (high > 16) {
            topack = 1;
            comb = malloc(topack * sizeof(*comb));
            comb[0] = 32;
            return comb;
        } else {
            topack = 2;
            comb = malloc(topack * sizeof(*comb));
            comb[0] = 16;
            comb[1] = 16;
            return comb;
        }
    }
}


void print_selector_table(selector table[])
{
    int i;
    int rows;

    rows = 256; /* using 8 bit selector */
    
    printf("selector table:\n");
    for (i = 0; i < rows; i++) {
        printf("ints to pack: %d, Bitwidths: ", table[i].intstopack);
        for (int j = 0; j < table[i].intstopack; j++) {
            printf("%d, ", table[i].bitwidths[j]);
        }
        printf("\n");
    }
}


/* getStats() calculates statistics of a list for use in selector generator
 returns a listStats structure, takes list number and length */
listStats getStats(int number, int length)
{
    int i, prev, max, mode, lowexception, nintyfifth, set95th;
    int highoutliers, lowoutliers;
    int *bitwidths, *dgaps;
    double sum, mean, stdev, fraction;
    listStats tempList;
    tempList.listNumber = number;
    tempList.listLength = length;
    
    bitwidths = malloc(32 * sizeof(bitwidths[0]));
    for (i = 0; i < 32; i++) {
        bitwidths[i] = 0;
    }
    
    /* convert docnums to dgaps and fill bitwidths frequency array */
    dgaps = malloc(length * sizeof(dgaps[0]));
    prev = 0;
    for (i = 0; i < length; i++) {
        dgaps[i] = postings_list[i] - prev;
        prev = postings_list[i];
        bitwidths[fls(dgaps[i])]++;
    }
    
    /* calculate mean and standard deviation */
    sum = 0;
    stdev = 0;
    for (i = 0; i < length; i++) {
        sum += fls(dgaps[i]);
    }
    mean = sum / length;
    for (i = 0; i < length; i++) {
        stdev += pow(fls(dgaps[i]) - mean, 2);
    }
    stdev = sqrt(stdev/length);
    tempList.mean = mean;
    tempList.stdev = stdev;
    
    sum = 0;
    fraction = 0;
    max = 0;
    set95th = 0;
    highoutliers = 0, lowoutliers = 0;
    
    /* find mode and 95th percentile */
    for (i = 0; i < 32; i++) {
        sum += bitwidths[i];
        fraction = (double) sum / length;
        if (bitwidths[i] >= max) {
            max = bitwidths[i];
            mode = i;
        }
        if (set95th == 0 && fraction >= 0.95) {
            set95th = 1;
            nintyfifth = i;
        }
    }
    
    /* find exception frequencies */
    for (i = 0; i < mode; i++) {
        lowoutliers += bitwidths[i];
    }
    for (i = mode + 1; i < 32; i++) {
        highoutliers += bitwidths[i];
    }
    
    /* find next most frequent bitwidth smaller than the mode */
    max = 0;
    for (i = 0; i < mode; i++) {
        if (bitwidths[i] >= max) {
            max = bitwidths[i];
            lowexception = i;
        }
    }
    
    tempList.mode = mode;
    tempList.lowexcp = lowexception;
    tempList.highexcp = nintyfifth;
    tempList.modFrac = (double) bitwidths[mode] / length;
    tempList.lowFrac = (double) lowoutliers / length;
    tempList.highFrac = (double) highoutliers / length;
    
    return tempList;
}


int main(int argc, char *argv[])
{
    int listnumber, i;
    const char *filename;
    FILE *fp;
    uint32_t length;
    listStats stats;
    
    if (argc == 2) {
        filename = argv[1];
    } else {
        exit(printf("Usage::%s <binfile>\n", argv[0]));
    }
    
    postings_list = malloc(NUMBER_OF_DOCS * sizeof postings_list[0]);
    
    if ((fp = fopen(filename, "rb")) == NULL) {
        exit(printf("Cannot open %s\n", filename));
    }
    
    listnumber = 0;
    while (fread(&length, sizeof(length), 1, fp)  == 1) {
        
        /* Read one postings list (and make sure we did so successfully) */
        if (fread(postings_list,
                  sizeof(*postings_list), length, fp) != length) {
            exit(printf("i/o error\n"));
        }
        listnumber++;
        
        stats = getStats(listnumber, length);
        
        comb = make_combs(stats.mode, stats.modFrac, stats.lowexcp,
                          stats.lowFrac, stats.highexcp, stats.highFrac);
        
        if (listnumber == 45979) {
            rowsfilled = 0;
            
            printf("mode of list 96: %d\n", stats.mode);
            for (i = 0; i <= stats.mode; i++) {
                int bitwidth = i + 1;
                int numbertopack = 32 / bitwidth;
                table[i].intstopack = numbertopack;
                table[i].bitwidths = malloc(numbertopack *
                                            sizeof(*table[i].bitwidths));
                for (int j = 0; j < numbertopack; j++) {
                    table[i].bitwidths[j] = bitwidth;
                }
            }
            rowsfilled = i;
            plainrows = i;
            printf("plain rows: %d\n", plainrows);
            generate_perms(comb, topack, add_perm_to_table);

            for (i = 6; i > 0; i--) {
                table[rowsfilled].intstopack = i;
                int bitwidth = 32 / i;
                table[rowsfilled].bitwidths = malloc(i * sizeof(*table[rowsfilled].bitwidths));
                for (int j = 0; j < i; j++) {
                    table[rowsfilled].bitwidths[j] = bitwidth;
                }
                rowsfilled++;
                plainrows++; /* just for sanity check */
                
            }
            printf("plain rows: %d\n", plainrows);
            
            
            print_selector_table(table);
        }

    }/* end read-in of a single list*/
    
    printf("number of lists: %d\n", listnumber);
    
    return 0;
}

