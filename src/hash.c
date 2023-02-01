#include <string.h>
#include "hash.h"

unsigned int hash(char *name, unsigned int size)
{
    int length,i;
    unsigned int hash_value = 0;

    length = strlen(name);
    for(i = 0; i < length; i++)
    {
        hash_value += name[i];
        hash_value = (hash_value * name[i] + length) % size;
    }
    return hash_value;
}