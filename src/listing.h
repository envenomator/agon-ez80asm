#ifndef LISTING_H
#define LISTING_H

#include "./stdint.h"
#include "config.h"

void listInit(void);
void listStartLine(char *line);
void listEndLine(void);
void listEmit8bit(uint8_t value);

#endif // LISTING_H
