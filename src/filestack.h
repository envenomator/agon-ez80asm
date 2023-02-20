#ifndef FILESTACK_H
#define FILESTACK_H

#include <stdint.h>
#include <stdbool.h>
#include "globals.h"

#define FILESTACK_MAXFILES  16

typedef struct {
    uint32_t address;
    FILE *fp;
} filestacktype;

void filestackInit(void);
uint8_t filestackCount(void);
bool filestackPush(filestacktype *fs);
bool filestackPop(filestacktype *fs);

#endif