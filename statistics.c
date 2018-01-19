#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#define NUMBER_OF_DOCS (1024 * 1024 * 128)

static uint32_t *postings_list;

int numperms = 0; /* number of distinct permutations of a bitwidth combination */
int *comb; /* bitwidth combination array generated for each list */
int topack; /* global variable to hold number of ints in the generated combination */


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



/* return 1 if a bigger, -1 if b bigger, 0 if equal */
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
        //printf("%d%c", array[i], i == length - 1 ? '\n' : ' ');
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
    } while (next_lex_perm(x, n));
}


/* make combination selector for a list with given statistics */
int * make_combs(int mode, double modeFrac, int low, double lowFrac, int high, double highFrac)
{
    int bitsused, numInts, i;

    int payload = 32;
    int numLow = 1, numMode = 1, numHigh = 1;
    double lowusedfrac, modeusedfrac, highusedfrac;

    if (low == 0) {
        low = 1;
    }
    
    /* count correct number of ints of each bitwidth to use */
    if (low + high + mode <= 32) {
        numInts = numLow + numMode + numHigh;
        bitsused = low + mode + high;
        lowusedfrac = (double) numLow / numInts; 
        modeusedfrac = (double) numMode / numInts;
        highusedfrac = (double) numHigh / numInts;
        int tries = 0;
        while (bitsused < 32) {
            tries++;
            if (highusedfrac < highFrac && bitsused + high <= 32) {
                numHigh++;
                bitsused += high;
                numInts++;
                highusedfrac = (double) numHigh / numInts;
            }
            if (modeusedfrac < modeFrac && bitsused + mode <= 32) {
                numMode++;
                bitsused += mode;
                numInts++;
                modeusedfrac = (double) numMode / numInts;
            }
            if (lowusedfrac < lowFrac && bitsused + low <= 32) {
                numLow++;
                bitsused += low;
                numInts++;
                lowusedfrac = (double) numLow / numInts;
            }
            /* most cases break out of loop here, some get stuck because of fractions */
            if (bitsused + low > 32 && bitsused + high - mode > 32 && bitsused + mode - low > 32) {
                break;
            }
            /* add extra ints to fill space regardless of modfrac, catches a few
               cases that can't break out because of exception frequencies */
            /****** 20 is a made up number and is probably bigger than it needs to be  ******/
            if (tries > 20) {
                /* best option is to promote one mode to high exception
                   (the order of these if elses is a judgement call i've made) */
                if (bitsused + high - mode <= 32 && numMode > 0) {
                    numMode--;
                    numHigh++;
                    bitsused += high - mode;
                }
                /* next best is to add a mode */
                if (bitsused + mode <= 32) {
                    numMode++;
                    bitsused += mode;
                }
                /* else try promote a low to a mode */
                if (bitsused + mode - low <= 32) {
                    numLow--;
                    numMode++;
                    bitsused += mode - low;
                }
                /* last option is to add a low exception */
                if (bitsused + low <= 32) {
                    numLow++;
                    bitsused += low;
                } else {
                    break; 
                }
            }
        }
  
        /* check for errors: pipe output to grep wasted or grep overflow */
        int external_wasted_bits = 32 - numLow * low - numMode * mode - numHigh * high;
        if (external_wasted_bits > low) {
            printf("%d wasted bits  ", external_wasted_bits);
        }
        if (bitsused > 32) {
            printf("overflow  ");
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
        
        topack = numInts; /* send number of ints in combination to global variable */
        qsort(comb, topack, sizeof(*comb), compare_ints);
        return comb;

    } else {
        /* deal with case where high + low + mode > 32 */
        /* currently just have two options, pack 1 or 2 ints */
        /* may want to do something slightly fancier */
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


/* getStats function calculate statistics of a list for use in selector generator
   returns a listStats structure, takes list number and length */
listStats getStats(int number, int length)
{
    listStats tempList;
    tempList.listNumber = number;
    tempList.listLength = length;

    int i;
    int *bitwidths = malloc(32 * sizeof(bitwidths[0]));
    for (i = 0; i < 32; i++) {
        bitwidths[i] = 0;
    }

    /* convert docnums to dgaps and fill bitwidths frequency array */
    int *dgaps = malloc(length * sizeof(dgaps[0]));
    int prev = 0;
    for (i = 0; i < length; i++) {
        dgaps[i] = postings_list[i] - prev;
        prev = postings_list[i];
        bitwidths[fls(dgaps[i])]++;
    }

    /* calculate mean and standard deviation */
    double sum = 0;
    double mean;
    double stdev = 0;
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
    double fraction = 0;
    int max = 0;
    int mode;
    int lowexception;
    int nintyfifth;
    int set95th = 0;
    int highoutliers = 0, lowoutliers = 0;

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
    int i, listnumber = 0;
   
    const char *filename;

    if (argc == 2) {
        filename = argv[1];
    } else {
        exit(printf("Usage::%s <binfile>\n", argv[0]));
    }

    postings_list = malloc(NUMBER_OF_DOCS * sizeof postings_list[0]);

    //printf("Using: %s\n", filename);
    FILE *fp;
    if ((fp = fopen(filename, "rb")) == NULL) {
	exit(printf("Cannot open %s\n", filename));
    }

    int selectorsize;
    int * selectorbits = malloc(32 * sizeof(*selectorbits));
    memset(selectorbits, 0, 32 * sizeof(*selectorbits)); 
    
    uint32_t length;
    while (fread(&length, sizeof(length), 1, fp)  == 1) {

        /* Read one postings list (and make sure we did so successfully) */
        if (fread(postings_list, sizeof(*postings_list), length, fp) != length) {
            exit(printf("i/o error\n"));
        }
        //printf("%u: ", (unsigned)length);
        listnumber++;

        int perms;
            
        listStats stats = getStats(listnumber, length);
        
        comb = make_combs(stats.mode, stats.modFrac, stats.lowexcp, stats.lowFrac, stats.highexcp, stats.highFrac);
                
        numperms = 0;
        //qsort(comb, topack, sizeof(*comb), compare_ints);
        /* generate_perms has the side effect of reverse sorting the bitwidth array */
        generate_perms(comb, topack, output_perms); /* this updates global variable numperms */
        stats.numPerms = numperms;

        if (numperms > 256) {
            for (i = topack - 1; i >= 0; i--) {
                printf("%d, ", comb[i]);
            }
            printf(" number of permutations: %d", numperms);
            printf("\n");
        }

        /* print statistics to a csv file to look at with matlab */
        //printf("%d, %4d,  %.3f, %.3f, %2d, %2d, %2d, %.5f, %.5f, %.5f, %d\n", listnumber, length, stats.mean, stats.stdev, stats.mode, stats.lowexcp, stats.highexcp, stats.modFrac, stats.lowFrac, stats.highFrac, stats.numPerms);

        /* add selector size needed for this list to selector bit frequency array */
        selectorsize = fls(stats.numPerms);
        selectorbits[selectorsize]++;
        
        //}/* end single list stats stuff */
             
    }/* end read-in of a single list*/

    /* print selector size stats */
    for (i = 1; i < 32; i++) {
        printf("%5d %5d\n", i, selectorbits[i]);
    }

    int eightbitselectorsum = 0;
    for (i = 1; i < 9; i++) {
        eightbitselectorsum += selectorbits[i];
    }

    int bigselectors = 0;
    for (i = 9; i < 32; i++) {
        bigselectors += selectorbits[i];
    }

    double frac8bit = (double) eightbitselectorsum / (eightbitselectorsum + bigselectors);
    printf("%.5f\n", frac8bit);
    
    return 0;
}
