#ifndef FILESTACK_H
#define FILESTACK_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "globals.h"
#include "filestack.h"
#include "utils.h"

typedef struct {
    uint16_t linenumber;
    FILE*    fp;
    char     filename[FILENAMEMAXLENGTH + 1];
    char *   filebuffer;
    char *   bufferstart;
    uint24_t filebuffersize;
    bool     fileEOF;
} filestackitem;

void filestackInit(void);
uint8_t filestackCount(void);
bool filestackPush(filestackitem *fs);
bool filestackPop(filestackitem *fs);

#endif