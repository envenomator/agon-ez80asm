#ifndef IO_H
#define IO_H

#include "config.h"

// Global variables
extern bool list_enabled;
extern bool consolelist_enabled;
extern char filename[FILES][FILENAMEMAXLENGTH];    // 0 - current, 1 - input, 2 - binary output, 3 - local labels, 4 - anonymous labels, 5 - listing
extern uint8_t filehandle[FILES];

enum {
    FILE_DELETELIST,
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
char  io_getc(uint8_t fh);                           // buffered read of a single byte / fallback
char* io_gets(uint8_t fh, char *s, int size);        // buffered read of a string / fallback
void  io_addDeleteList(char *name);                  // add name of file to delete list at io_close

void getMacroFilename(char *filename, char *macroname);

//char *agon_fgets(char *s, int size, uint8_t fileid);

#endif // IO_H
