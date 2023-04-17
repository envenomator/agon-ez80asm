#ifndef FILESTACK_H
#define FILESTACK_H

#include "./stdint.h"
#include "globals.h"
#include "config.h"

typedef struct {
    uint16_t linenumber;
    uint8_t  fp;
    char     filename[FILENAMEMAXLENGTH];
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