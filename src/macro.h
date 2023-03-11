#ifndef MACRO_H
#define MACRO_H

#include <string.h>
#include <stdio.h>
#include "./stdint.h"
#include "config.h"
#include "utils.h"
#include "globals.h"
#include "malloc.h"

typedef struct {
    char*   name;
    uint8_t argcount;
    char**  arguments;
} macro;

void    initMacros(void);
bool    defineMacro(char *name, uint8_t argcount, char *arguments);
macro * findMacro(char *name);

#endif // MACRO_H
