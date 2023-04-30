#include "mos_posix.h"
#include "./stdint.h"

uint8_t _fileindex;
uint8_t _mosfileid;

void mos_posix_init(void) {
    _fileindex = 0;
    _mosfileid = 1;
}

#ifndef AGON

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mos-interface.h"

typedef struct {
    FILE *file;
    uint8_t mosfile;
} posixfile;

#define MAXPOSIXFILES 256
posixfile _filearray[MAXPOSIXFILES];

// MOS API calls conversion to POSIX
UINT8 mos_fopen(char * filename, UINT8 mode) // returns filehandle, or 0 on error
{
    posixfile newfile;

    if(_fileindex < (MAXPOSIXFILES - 1)) {
        if(_mosfileid == 255) return 0;
        newfile.file = NULL;
        if(mode == fa_read) newfile.file = fopen(filename, "r");
        if(mode == (fa_write | fa_create_always)) newfile.file = fopen(filename, "wb+");
        if(newfile.file == NULL) return 0;
        newfile.mosfile = _mosfileid++;
        _filearray[_fileindex++] = newfile;
        return newfile.mosfile;
    }
    else return 0;
}

UINT8 mos_fclose(UINT8 fh)					 // returns number of still open files
{
    int index;
    bool found = false;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) {
        fclose(_filearray[index].file);
        for(;index < _fileindex - 1; index++) {
            _filearray[index] = _filearray[index+1];
        }
        _fileindex--;
        return 1;
    }
    return 0;
}

char mos_fgetc(UINT8 fh)					 // returns character from file
{
    int index;
    bool found = false;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) return fgetc(_filearray[index].file);
    else return 0;
}

void mos_fputc(UINT8 fh, char c)			 // writes character to file
{
    int index;
    bool found = false;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) fputc(c, _filearray[index].file);
}

UINT8 mos_feof(UINT8 fh)					 // returns 1 if EOF, 0 otherwise
{
    int index;
    bool found = false;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) return feof(_filearray[index].file);
    else return 1;
}

UINT24 mos_fwrite(UINT8 fh, char *buffer, UINT24 numbytes) {
    int index;
    bool found = false;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) return fwrite(buffer, numbytes, 1, _filearray[index].file);
    else return 0;
}

UINT24 mos_fread(UINT8 fh, char *buffer, UINT24 numbytes) {
    int index;
    bool found = false;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) return fread(buffer, 1, numbytes, _filearray[index].file);
    else return 0;
}

UINT8 mos_del(char *filename) {
    remove(filename);
    return 0;
}


int putch(int a) {
    return putchar(a);
}

#endif