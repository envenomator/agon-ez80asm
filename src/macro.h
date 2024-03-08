#ifndef MACRO_H
#define MACRO_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "utils.h"

typedef struct {
    char*   name;
    char*   body;
    uint8_t argcount;
    char**  arguments;
    char**  substitutions;
} macro_t;

extern macro_t macroTable[MAXIMUM_MACROS]; // indexed table
extern uint8_t macroTableCounter;
extern uint24_t macromemsize;

void      initMacros(void);
macro_t * defineMacro(char *name, uint8_t argcount, char *arguments);
macro_t * findMacro(char *name);
void      setMacroBody(macro_t *macro, const char *body);
uint8_t   macroExpandArg(char *dst, char *src, macro_t *m);
#endif // MACRO_H
