#ifndef MACRO_H
#define MACRO_H

#include "defines.h"

extern uint8_t macroCounter;
extern uint24_t macromemsize;
extern uint24_t macroExpandID;

void      initMacros(void);
bool      defineMacro(char *definition, struct contentitem *ci);
uint8_t   macroExpandArg(char *dst, const char *src, const macro_t *m);
bool      readMacroBody(struct contentitem *ci);
bool      parseMacroArguments(macro_t *macro, char *invocation, char (*substitutionlist)[MACROARGSUBSTITUTIONLENGTH + 1]);
#endif // MACRO_H
