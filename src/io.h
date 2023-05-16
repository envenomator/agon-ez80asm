#ifndef IO_H
#define IO_H

#include "config.h"
#include "filestack.h"

// Global variables
extern bool list_enabled;
extern bool consolelist_enabled;
extern char filename[FILES][FILENAMEMAXLENGTH];    // 0 - current, 1 - input, 2 - binary output, 3 - local labels, 4 - anonymous labels, 5 - listing
extern uint8_t filehandle[FILES];

enum {
    FILE_CURRENT,
    FILE_INPUT,
    FILE_OUTPUT,
    FILE_LOCAL_LABELS,
    FILE_ANONYMOUS_LABELS,
    FILE_LISTING,
    FILE_MACRO
};

bool  io_init(char *input_filename);                 // init - called once at start
bool  io_setpass(uint8_t pass);                      // assembly pass, set needed state of files
void  io_close(void);                                // close everything at end, do cleanup
void  io_putc(uint8_t fh, unsigned char c);          // buffered write of a single byte / fallback
int   io_puts(uint8_t fh, char *s);                  // buffered write of a string / fallback
char* io_gets(uint8_t fh, char *s, int size);        // buffered read of a string / fallback
void  io_getFileDefaults(filestackitem *fsi);        // set file defaults (traditional mode) to fsi object
void  io_getCurrent(filestackitem *fsi);             // retrieve CURRENT as fsi
void  io_setCurrent(filestackitem *fsi);             // set CURRENT from fsi
void  io_resetCurrentInput(void);                    // set FILE_CURRENT to FILE_INPUT specs
void  io_getMacroFilename(char *filename, char *macroname);

#endif // IO_H
