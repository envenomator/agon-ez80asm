#include <stdio.h>
#include <string.h>
#include "listing.h"
#include "globals.h"
#include "utils.h"
#include "stdint.h"

char _listLine[LINEMAX];
uint24_t _listAddress;
uint8_t _listObjects[256];
uint8_t _listObjectCount;
bool _listFirstline = true;

char _listHeader[] = "PC     Output            Line Source\n\r";

char buffer[LINEMAX + 32];

void listInit(bool console) {
    sprintf(buffer, "%s", _listHeader);
    agon_fputs(buffer, FILE_LISTING);
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
}

void listEndLine(bool console) {
    uint8_t i, lines, objectnum;
    uint8_t linemax;

    linemax = (_listObjectCount / OBJECTS_PER_LINE);
    if(_listObjectCount % OBJECTS_PER_LINE) linemax ++;
    if(linemax == 0) linemax = 1;

    objectnum = 0;
    for(lines = 0; lines < linemax; lines++) {
        if(lines == 0) {
            sprintf(buffer, "%06X ",_listAddress);
            agon_fputs(buffer, FILE_LISTING);
            if(console) printf("%s",buffer);
        }
        else {
            sprintf(buffer, "       ");
            agon_fputs(buffer, FILE_LISTING);
            if(console) printf("%s",buffer);
        }
        for(i = 0; i < OBJECTS_PER_LINE; i++) {
            if(objectnum < _listObjectCount) {
                sprintf(buffer, "%02X ",_listObjects[objectnum]);
                agon_fputs(buffer, FILE_LISTING);
                if(console) printf("%s",buffer);
                objectnum++;
            }
            else {
                sprintf(buffer, "   ");
                agon_fputs(buffer, FILE_LISTING);
                if(console) printf("%s",buffer);
            }
        }
        if(lines == 0) {
            sprintf(buffer, "%04d %s\n\r",linenumber, _listLine);
            agon_fputs(buffer, FILE_LISTING);
            if(console) printf("%s",buffer);
        }
        else {
            sprintf(buffer, "\n\r");
            agon_fputs(buffer, FILE_LISTING);
            if(console) printf("%s",buffer);
        }
    }
}

void listEmit8bit(uint8_t value) {
    _listObjects[_listObjectCount++] = value; 
}