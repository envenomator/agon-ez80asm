#ifndef LISTING_H
#define LISTING_H

#include <stdint.h>

#define OBJECTS_PER_LINE 6

void listInit(bool output_console);
void listStartLine(char *line);
void listEndLine(bool output_console);
void listEmit8bit(uint8_t value);

#endif // LISTING_H
