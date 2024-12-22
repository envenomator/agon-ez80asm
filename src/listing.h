#ifndef LISTING_H
#define LISTING_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "listing.h"
#include "globals.h"
#include "utils.h"
#include "io.h"

void listInit(void);
void listStartLine(char *line, unsigned int linenumber);
void listEndLine(void);
void listPrintDSLines(int number, int value);
void listEmit8bit(uint8_t value);
void listPrintComment(const char *src);

#endif // LISTING_H
