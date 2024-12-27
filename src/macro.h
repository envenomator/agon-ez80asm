#ifndef MACRO_H
#define MACRO_H

#include "defines.h"

extern uint8_t macroCounter;
extern uint24_t macromemsize;
extern uint24_t macroExpandID;

void      initMacros(void);
macro_t * defineMacro(const char *name, uint8_t argcount, const char *arguments, uint16_t startlinenumber);
uint8_t   macroExpandArg(char *dst, const char *src, const macro_t *m);
bool      readMacroBody2Buffer(void);
bool      parseMacroInvocation(char *str, char **name, uint8_t *argcount, char *arglist);

#endif // MACRO_H
