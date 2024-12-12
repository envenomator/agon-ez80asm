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
    char*   originfilename;
   uint24_t originlinenumber;
    char*   body;
    uint8_t argcount;
    char**  arguments;
    char**  substitutions;
    void*   next;
   uint24_t currentExpandID;
} macro_t;

extern uint8_t macroCounter;
extern uint24_t macromemsize;
extern uint24_t macroExpandID;

void      initMacros(void);
macro_t * defineMacro(char *name, uint8_t argcount, char *arguments, uint16_t startlinenumber);
void      setMacroBody(macro_t *macro, const char *body);
uint8_t   macroExpandArg(char *dst, char *src, macro_t *m);
#endif // MACRO_H
