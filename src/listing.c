#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "defines.h"
#include "listing.h"
#include "globals.h"
#include "utils.h"
#include "io.h"
#include "assemble.h"

// Local variables
char     _listLine[LINEMAX+1];
uint24_t _listAddress;
uint8_t  _listObjects[LISTING_OBJECTS_PER_LINE];
uint8_t  _listLineObjectCount;
uint16_t  _listLineNumber;
uint24_t _listSourceLineNumber;

char _listHeader[]     = "PC     Output      Line\n\r";
char _listDataHeader[] = "       ";

char buffer[(LINEMAX * 2) + 1];

void listInit(void) {
    sprintf(buffer, "%s", _listHeader);
    if(list_enabled) ioPuts(FILE_LISTING, buffer);
    if(consolelist_enabled) printf("%s", _listHeader);
    _listLine[0] = 0;
}

void listStartLine(const char *line, unsigned int linenumber) {    
    strcpy(_listLine, line);
    trimRight(_listLine);
    _listAddress = address;
    _listLineObjectCount = 0;
    _listSourceLineNumber = currentExpandedMacro?_listSourceLineNumber:linenumber; // remember upstream linenumber during macro expansion
    _listLineObjectCount = 0;
    _listLineNumber = 0;
}

void listPrintDSLines(int number, int value) {
    while(number) {
        uint8_t i = 0;
        if(list_enabled) ioPuts(FILE_LISTING, _listDataHeader);
        if(consolelist_enabled) printf("%s",_listDataHeader);

        while(i < LISTING_OBJECTS_PER_LINE) {
            if(number) {
                sprintf(buffer, "%02X ",value);
                if(list_enabled) ioPuts(FILE_LISTING, buffer);
                if(consolelist_enabled) printf("%s",buffer);
                number--;
            }
            i++;
        }
        if(list_enabled) ioPuts(FILE_LISTING, "\n");
        if(consolelist_enabled) printf("\n");
    }
}

void listPrintLine(void) {
    uint8_t i,spaces;

    if(_listLineNumber == 0) {
        sprintf(buffer, "%06X ",_listAddress);
        if(list_enabled) ioPuts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    else {
        if(list_enabled) ioPuts(FILE_LISTING, _listDataHeader);
        if(consolelist_enabled) printf("%s", _listDataHeader);
    }
    for(i = 0; i < _listLineObjectCount; i++) {
        sprintf(buffer, "%02X ",_listObjects[i]);
        if(list_enabled) ioPuts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    spaces = LISTING_OBJECTS_PER_LINE - _listLineObjectCount;
    for(i = 0; i < spaces; i++) {
        sprintf(buffer, "   ");
        if(list_enabled) ioPuts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }
    if(_listLineNumber == 0) {
        sprintf(buffer, "%04d", currentExpandedMacro?macrolinenumber:_listSourceLineNumber);
        for(i = 1; i < contentlevel; i++) {
            strcat(buffer, "*");
        }
        if(macroexpansions) {
            if(currentExpandedMacro) {
                char tmpbuffer[6];
                snprintf(tmpbuffer, 6, "M%d ", macrolevel);
                strcat(buffer, tmpbuffer);
            }
            else strcat(buffer, "   ");
        }
        else strcat(buffer, " "); // single space between line# and input line, if no macros expanded

        for(i = maxstackdepth - i;i > 0; i--) {
            strcat(buffer, " ");
        }
        if(list_enabled) ioPuts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
        sprintf(buffer, "%s", _listLine);
        if(list_enabled) ioPuts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
    }

    if(list_enabled) ioPuts(FILE_LISTING, "\n");
    if(consolelist_enabled) printf("\n");

    _listLineObjectCount = 0;
    _listLineNumber++;
}

void listPrintComment(const char *src) {
        sprintf(buffer, "                       M%d %s\n", macrolevel, src);
        if(list_enabled) ioPuts(FILE_LISTING, buffer);
        if(consolelist_enabled) printf("%s",buffer);
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