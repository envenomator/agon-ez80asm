#include <stdio.h>
#include <string.h>
#include "listing.h"
#include "globals.h"
#include "utils.h"
#include "./stdint.h"
#include "io.h"

// Global variables
bool list_enabled;
bool consolelist_enabled;

// Local variables
char _listLine[LINEMAX];
uint24_t _listAddress;
uint8_t _listObjects[256];
uint8_t _listObjectCount;
bool _listFirstline = true;
uint24_t _listLinenumber;
bool _expandedmacro;

char _listHeader[] = "PC     Output            Line Source\n\r";

char buffer[LINEMAX + 32];

void listInit(bool console) {
    sprintf(buffer, "%s", _listHeader);
    io_puts(FILE_LISTING, buffer);
    if(console) printf("%s", _listHeader);
    _listFirstline = false;
    _listLine[0] = 0;
    _listObjectCount = 0;
}

void listStartLine(char *line) {    
    strcpy(_listLine, line);
    trimRight(_listLine);
    _listAddress = address;
    _listObjectCount = 0;
    _listLinenumber = currentExpandedMacro?_listLinenumber:linenumber; // remember upstream linenumber during macro expansion
    _expandedmacro = (currentExpandedMacro != NULL);
}

void listEndLine(bool console) {
    uint8_t i, lines, objectnum;
    uint8_t linemax;

    linemax = (_listObjectCount / LISTING_OBJECTS_PER_LINE);
    if(_listObjectCount % LISTING_OBJECTS_PER_LINE) linemax ++;
    if(linemax == 0) linemax = 1;

    objectnum = 0;
    for(lines = 0; lines < linemax; lines++) {
        if(lines == 0) {
            sprintf(buffer, "%06X ",_listAddress);
            io_puts(FILE_LISTING, buffer);
            if(console) printf("%s",buffer);
        }
        else {
            sprintf(buffer, "       ");
            io_puts(FILE_LISTING, buffer);
            if(console) printf("%s",buffer);
        }
        for(i = 0; i < LISTING_OBJECTS_PER_LINE; i++) {
            if(objectnum < _listObjectCount) {
                sprintf(buffer, "%02X ",_listObjects[objectnum]);
                io_puts(FILE_LISTING, buffer);
                if(console) printf("%s",buffer);
                objectnum++;
            }
            else {
                sprintf(buffer, "   ");
                io_puts(FILE_LISTING, buffer);
                if(console) printf("%s",buffer);
            }
        }
        if(lines == 0) {
            sprintf(buffer, "%04d %s\r\n",_listLinenumber, _listLine);
            io_puts(FILE_LISTING, buffer);
            if(console) printf("%s",buffer);
        }
        else {
            sprintf(buffer, "\r\n");
            io_puts(FILE_LISTING, buffer);
            if(console) printf("%s",buffer);
        }
    }
}

void listEmit8bit(uint8_t value) {
    _listObjects[_listObjectCount++] = value; 
}