#ifndef STR2NUM_H
#define STR2NUM_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

// Transforms a binary/hexadecimal/decimal string to an int32_t number
// option to NOT halt on errors, but just set the err_str2num status - just check if something is a valid number
int32_t str2num(char *string, bool errorhalt);

extern bool err_str2num;

#endif // STR2NUM_H