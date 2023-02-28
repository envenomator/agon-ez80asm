#include <string.h>
#include "hash.h"
#include "stdint.h"

unsigned int hash(char *name, unsigned int size) {
    unsigned int h;
    unsigned char *p;

    h = 7;
    for(p = (unsigned char*)name; *p != '\0'; p++) 
        h = 37 * h + *p;
    return h % size;
}