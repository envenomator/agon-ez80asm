#ifndef HASH_H
#define HASH_H

#include <string.h>
#include <ctype.h>
#include "hash.h"
#include <stdint.h>

uint16_t hash(char *key); // returns hash as 16bit unsigned int
uint16_t lowercaseHash(char *key);

#endif // HASH_H