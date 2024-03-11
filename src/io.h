#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "globals.h"
#include "listing.h"
#include "macro.h"
#include "utils.h"
#include "moscalls.h"

// Global variables
extern bool list_enabled;
extern bool consolelist_enabled;
extern char filename[FILES][FILENAMEMAXLENGTH + 1];    // 0 - current, 1 - input, 2 - binary output, 3 - anonymous labels, 4 - listing, 5 - symbols
extern FILE* filehandle[FILES];
extern char filelabelscope[FILES][FILENAMEMAXLENGTH + 1];
extern uint16_t sourcefilecount;
extern uint16_t binfilecount;

enum {
    FILE_CURRENT,
    FILE_INPUT,
    FILE_OUTPUT,
    FILE_ANONYMOUS_LABELS,
    FILE_LISTING,
    FILE_SYMBOLS
};

//#ifdef AGON
//extern unsigned int io_filesize(uint8_t fh);
//#endif

FILE *io_openfile(char *name, char *mode);
uint24_t io_getfilesize(FILE *fh);

void io_outputc(unsigned char c);
void io_write(uint8_t fh, char *s, uint16_t size);
bool io_init(char *input_filename, char *output_filename); // init - called once at start
void io_close(void);                                // close everything at end, do cleanup
void io_putc(uint8_t fh, unsigned char c);          // buffered write of a single byte / fallback
int  io_puts(uint8_t fh, char *s);                  // buffered write of a string / fallback
void emit_8bit(uint8_t value);
void emit_16bit(uint16_t value);
void emit_24bit(uint24_t value);
void emit_32bit(uint32_t value);
void emit_quotedstring(char *str);
void emit_adlsuffix_code(uint8_t suffix);
void emit_immediate(operand_t *op, uint8_t suffix);

#endif // IO_H
