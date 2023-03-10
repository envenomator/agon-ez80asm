#ifndef MALLOC_H
#define MALLOC_H

#include "./stdint.h"
#include "config.h"

void init_agon_malloc(void);
char *agon_malloc(uint16_t length);

#endif // MALLOC_H