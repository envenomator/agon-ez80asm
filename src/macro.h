#ifndef MACRO_H
#define MACRO_H

#include <string.h>
#include <stdio.h>
#include "./stdint.h"
#include "config.h"
#include "utils.h"
#include "malloc.h"

typedef struct {
    char*   name;
    uint8_t argcount;
    char**  arguments;
    char**  substitutions;
} macro_t;

extern macro_t macroTable[MAXIMUM_MACROS]; // indexed table
extern uint8_t macroTableCounter;

void      initMacros(void);
bool      defineMacro(char *name, uint8_t argcount, char *arguments);
macro_t * findMacro(char *name);
//void      macroArgFindSubst(char *op, macro_t *m);
uint8_t   macroExpandArg(char *dst, char *src, macro_t *m);
#endif // MACRO_H
