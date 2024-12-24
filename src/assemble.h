#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include "config.h"
#include "defines.h"

extern uint24_t passmatchcounter;
void assemble(const char *filename);
void emit_8bit(uint8_t value);
void emit_16bit(uint16_t value);
void emit_24bit(uint24_t value);

struct contentitem *contentPop(void);
bool                contentPush(struct contentitem *ci);
struct contentitem *currentContent(void);
uint8_t             currentStackLevel(void);

#endif // ASSEMBLE_H