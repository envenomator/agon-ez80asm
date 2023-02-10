#ifndef STR2NUM_H
#define STR2NUM_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

// Transforms a binary/hexadecimal/decimal string to an uint32_t number
// Valid strings are
// BINARY:  0b..., ...b
// HEX:     0x..., ...h, $...
// DECIMAL ...
uint32_t str2num(char *string);

#endif // STR2NUM_H