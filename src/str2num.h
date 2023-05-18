#ifndef STR2NUM_H
#define STR2NUM_H

#include "./stdint.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

// Transforms a binary/hexadecimal/decimal string to an int24_t number
// option to NOT halt on errors, but just set the err_str2num status - just check if something is a valid number
int24_t str2num(char *string, bool errorhalt);
int24_t str2hex(char *string);
extern bool err_str2num;

#endif // STR2NUM_H