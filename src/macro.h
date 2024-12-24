#ifndef MACRO_H
#define MACRO_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "typedefs.h"
#include "utils.h"

extern uint8_t macroCounter;
extern uint24_t macromemsize;
extern uint24_t macroExpandID;

void      initMacros(void);
macro_t * defineMacro(const char *name, uint8_t argcount, const char *arguments, uint16_t startlinenumber);
void      setMacroBody(macro_t *macro, const char *body);
uint8_t   macroExpandArg(char *dst, const char *src, const macro_t *m);
#endif // MACRO_H
