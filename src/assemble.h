#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include "console.h"
#include "globals.h"
#include "utils.h"
#include "label.h"
#include "str2num.h"
#include "listing.h"
#include "macro.h"
#include "io.h"
#include "moscalls.h"

typedef enum {
    PS_START,
    PS_LABEL,
    PS_COMMAND,
    PS_OP,
    PS_COMMENT,
} parsestate;

struct contentitem {
    // Static items
    char         *name;
    unsigned int  size;
    char         *buffer;
    FILE         *fh;
    bool          filebuffered;
    void         *next;
    // Items changed during processing
    char         *readptr;
    uint16_t      currentlinenumber;
    char         *currentline;
    char         *currenterrorline;
    char          labelscope[MAXNAMELENGTH+1];
    uint8_t       inConditionalSection;
};

extern uint24_t passmatchcounter;
void assemble(char *filename);
void emit_8bit(uint8_t value);
void emit_16bit(uint16_t value);
void emit_24bit(uint24_t value);

struct contentitem *contentPop(void);
bool                contentPush(struct contentitem *ci);
struct contentitem *currentContent(void);
uint8_t             currentStackLevel(void);

#endif // ASSEMBLE_H