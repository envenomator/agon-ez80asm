#ifndef FILESTACK_H
#define FILESTACK_H

#include <stdint.h>
#include <stdbool.h>
#include "globals.h"

#define FILESTACK_MAXFILES  16

typedef struct {
    uint32_t address;
    uint16_t linenumber;
    FILE *fp;
} filestackitem;

void filestackInit(void);
uint8_t filestackCount(void);
bool filestackPush(filestackitem *fs);
bool filestackPop(filestackitem *fs);

#endif