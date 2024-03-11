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
void listEmit8bit(uint8_t value);

#endif // LISTING_H
