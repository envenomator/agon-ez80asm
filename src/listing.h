#ifndef LISTING_H
#define LISTING_H

#include <stdint.h>

#define OBJECTS_PER_LINE 6

void listInit(void);
void listStartLine(char *line);
void listEndLine(void);
void listEmit8bit(uint8_t value);

#endif // LISTING_H
