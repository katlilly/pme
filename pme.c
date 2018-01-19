#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#define NUMBER_OF_DOCS (1024 * 1024 * 128)

static uint32_t *postings_list;


int main(int argc, char *argv[])
{
    printf("entered main\n");
    int listnumber = 0;
    
    const char *filename;
    
    if (argc == 2) {
        filename = argv[1];
    } else {
        exit(printf("Usage::%s <binfile>\n", argv[0]));
    }
    
    postings_list = malloc(NUMBER_OF_DOCS * sizeof postings_list[0]);
    
    FILE *fp;
    if ((fp = fopen(filename, "rb")) == NULL) {
        exit(printf("Cannot open %s\n", filename));
    }
    
    uint32_t length;
    while (fread(&length, sizeof(length), 1, fp)  == 1) {
        
        /* Read one postings list (and make sure we did so successfully) */
        if (fread(postings_list, sizeof(*postings_list), length, fp) != length) {
            exit(printf("i/o error\n"));
        }
        
        listnumber++;
        
        
    }/* end read-in of a single list*/
    
    printf("number of lists: %d\n", listnumber);
    
    return 0;
}

