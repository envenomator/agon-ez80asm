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
} posixfile_t;

#define MAXPOSIXFILES 256
posixfile_t _filearray[MAXPOSIXFILES];

// MOS API calls conversion to POSIX

uint8_t mos_flseek(uint8_t fh, uint32_t offset) {
    int index;
    bool found = false;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) return fseek(_filearray[index].file, 0, SEEK_SET);
    return 1; // non-zero on failure
}

uint8_t mos_fopen(char * filename, uint8_t mode) // returns filehandle, or 0 on error
{
    posixfile_t newfile;

    if(_fileindex < (MAXPOSIXFILES - 1)) {
        if(_mosfileid == 255) return 0;
        newfile.file = NULL;
        
        if(mode & fa_write) newfile.file = fopen(filename, "wb+");
        else newfile.file = fopen(filename, "r");        
        
        
        if(newfile.file == NULL) return 0;
        newfile.mosfile = _mosfileid++;
        _filearray[_fileindex++] = newfile;
        return newfile.mosfile;
    }
    else return 0;
}

uint8_t mos_fclose(uint8_t fh)					 // returns number of still open files
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
        _mosfileid--;
        return 1;
    }
    return 0;
}

char mos_fgetc(uint8_t fh)					 // returns character from file
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

void mos_fputc(uint8_t fh, char c)			 // writes character to file
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

uint8_t mos_feof(uint8_t fh)					 // returns 1 if EOF, 0 otherwise
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

uint24_t mos_fwrite(uint8_t fh, char *buffer, uint24_t numbytes) {
    int index;
    bool found = false;
    uint24_t written;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) {
        written = fwrite(buffer, 1, numbytes, _filearray[index].file);
        fflush(_filearray[index].file);
        return written;
    }
    else return 0;
}

uint24_t mos_fread(uint8_t fh, char *buffer, uint24_t numbytes) {
    int index;
    bool found = false;

    for(index = 0; index < _fileindex; index++) {
        if(_filearray[index].mosfile == fh) {
            found = true;
            break;
        }
    }
    if(found) return (fread(buffer, 1, numbytes, _filearray[index].file));
    else return 0;
}

uint8_t mos_del(char *filename) {
    remove(filename);
    return 0;
}


int putch(int a) {
    return putchar(a);
}

#endif