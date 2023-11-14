#include <stdio.h>
#include <string.h>
#include "listing.h"
#include "globals.h"
#include "utils.h"
#include "./stdint.h"
#include "io.h"

// Local variables
char     _listLine[LINEMAX + 1];
uint24_t _listAddress;
uint8_t  _listObjects[LISTING_OBJECTS_PER_LINE];
uint8_t  _listLineObjectCount;
uint16_t  _listLineNumber;
uint24_t _listSourceLineNumber;
//bool _expandedmacro;

char _listHeader[] = "PC     Output            Line Source\n\r";

char buffer[LINEMAX * 2];

void listInit(void) {
    sprintf(buffer, "%s", _listHeader);
    if(list_enabled) io_puts(FILE_LISTING, buffer);
    if(consolelist_enabled) printf("%s", _listHeader);
    _listLine[0] = 0;
}

void listStartLine(char *line) {    
    strcpy(_listLine, line);
    trimRight(_listLine);
    _listAddress = address;
    _listLineObjectCount = 0;
    _listSourceLineNumber = currentExpandedMacro?_listSourceLineNumber:linenumber; // remember upstream linenumber during macro expansion
    //_expandedmacro = (currentExpandedMacro != NULL);
    _listLineObjectCount = 0;
    _listLineNumber = 0;
}

void listPrintLine(void) {
    uint8_t i,spaces;

    if(_listLineNumber == 0) {
        sprintf(buffer, "%06X ",_listAddress);
        if(list_enabled) io_puts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    else {
        sprintf(buffer, "       ");
        if(list_enabled) io_puts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    for(i = 0; i < _listLineObjectCount; i++) {
        sprintf(buffer, "%02X ",_listObjects[i]);
        if(list_enabled) io_puts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    spaces = LISTING_OBJECTS_PER_LINE - _listLineObjectCount;
    for(i = 0; i < spaces; i++) {
        sprintf(buffer, "   ");
        if(list_enabled) io_puts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    if(_listLineNumber == 0) {
        sprintf(buffer, "%04d %s\r\n",_listSourceLineNumber, _listLine);
        if(list_enabled) io_puts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    else {
        sprintf(buffer, "\r\n");
        if(list_enabled) io_puts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    _listLineObjectCount = 0;
    _listLineNumber++;
}

void listEndLine(void) {
    if(_listLineNumber == 0) listPrintLine(); // unfinished first line
    if((_listLineNumber) && (_listLineObjectCount)) listPrintLine(); // unfinished last line
}

void listEmit8bit(uint8_t value) {
    if(_listLineObjectCount == LISTING_OBJECTS_PER_LINE) {
        listPrintLine();
    }
    _listObjects[_listLineObjectCount++] = value; 
}