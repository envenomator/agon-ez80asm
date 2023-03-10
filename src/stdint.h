#ifndef STDINT_H
#define STDINT_H

#include "config.h"

#ifndef AGON
#include <stdint.h>
#include <stdbool.h>
typedef int8_t INT8;
typedef uint8_t UINT8;
typedef int int24_t;
typedef int INT24;
typedef unsigned int uint24_t;
typedef unsigned int UINT24;
#endif

#ifdef AGON
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int24_t;
typedef unsigned int uint24_t;
typedef long int32_t;
typedef unsigned long uint32_t;
typedef unsigned char bool;
typedef unsigned char byte;

#define true    1
#define TRUE    1
#define false   0
#define FALSE   0
#endif


#endif