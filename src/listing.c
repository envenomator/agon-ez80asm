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
    _expandedmacro = (currentExpandedMacro != NULL);
}

/*
void listReconstructLine(void) {
    char buffer[LINEMAX];
    
    if(currentExpandedMacro) {
        buffer[0] = 0;
        if(notEmpty(currentline.label)) {
            strcpy(buffer, currentline.label);
            strcat(buffer, ": ");
        }
        if(notEmpty(currentline.mnemonic)) {
            strcat(buffer, currentline.mnemonic);
            if(notEmpty(currentline.suffix)) {
                strcat(buffer, ".");
                strcat(buffer, currentline.suffix);
            }
            strcat(buffer, " ");
        }
        if(notEmpty(currentline.operand1)) {
            strcat(buffer, currentline.operand1);
        }
        if(notEmpty(currentline.operand2)) {
            strcat(buffer, ", ");
            strcat(buffer, currentline.operand2);
        }
        if(notEmpty(currentline.comment)) {
            strcat(buffer, " ");
            strcat(buffer, currentline.comment);
        }
        strcpy(_listLine, buffer);
    }
}
*/
void listEndLine(bool console) {
    uint8_t i, lines, objectnum;
    uint8_t linemax;

    //listReconstructLine();
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
            if(_expandedmacro)
                sprintf(buffer, "---- %s\r\n",_listLine);
            else
                sprintf(buffer, "%04d %s\r\n",linenumber, _listLine);
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