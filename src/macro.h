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
} macro;

void    initMacros(void);
bool    defineMacro(char *name, uint8_t argcount, char *arguments);
macro * findMacro(char *name);
void    macroArgFindSubst(char *op, macro *m);

#endif // MACRO_H
