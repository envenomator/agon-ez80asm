#ifndef STR2NUM_H
#define STR2NUM_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

extern bool err_str2num;
uint32_t str2num(char *string);

#endif // STR2NUM_H