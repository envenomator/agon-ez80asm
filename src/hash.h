#ifndef HASH_H
#define HASH_H

#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "hash.h"

//uint16_t hash(char *key); // returns hash as 16bit unsigned int
uint8_t hash256(const char *key);
uint8_t lowercaseHash256(const char *key);

#endif // HASH_H