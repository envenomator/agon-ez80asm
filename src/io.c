#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "./stdint.h"
#include "mos-interface.h"
#include "io.h"

// Global variables
char filename[FILES][FILENAMEMAXLENGTH];
uint8_t filehandle[FILES];

// Local variables
char _fileBasename[FILENAMEMAXLENGTH];
char _outputbuffer[OUTPUT_BUFFERSIZE];
unsigned int _outputbuffersize;


bool openFile(uint8_t *file, char *name, uint8_t mode) {
    *file = mos_fopen(name, mode);
    //printf("Opened file \"%s\", id %d\r\n", name, *file);
    if(*file) return true;
    //printf("Error opening \"%s\"\n\r", name);
    return false;
}

bool reOpenFile(uint8_t number, uint8_t mode) {
    bool result;
    //printf("Re-opening    id: %d\r\n",filehandle[number]);
    if(filehandle[number]) mos_fclose(filehandle[number]);
    result = openFile(&filehandle[number], filename[number], mode);
    //printf("Re-opened mos id: %d\r\n",filehandle[number]);
    return result;
}

void prepare_filenames(char *input_filename) {
    // prepare filenames
    strcpy(filename[FILE_INPUT], input_filename);
    strcpy(filename[FILE_OUTPUT], input_filename);
    remove_ext(filename[FILE_OUTPUT], '.', '/');
    strcpy(_fileBasename, filename[FILE_OUTPUT]);
    strcpy(filename[FILE_LOCAL_LABELS], _fileBasename);
    strcpy(filename[FILE_ANONYMOUS_LABELS],_fileBasename);
    if(list_enabled) strcpy(filename[FILE_LISTING],_fileBasename);
    strcpy(filename[FILE_DELETELIST],_fileBasename);
    strcat(filename[FILE_OUTPUT], ".bin");
    strcat(filename[FILE_LOCAL_LABELS], ".lcllbls");
    strcat(filename[FILE_ANONYMOUS_LABELS], ".anonlbls");
    if(list_enabled) strcat(filename[FILE_LISTING], ".lst");
    strcat(filename[FILE_DELETELIST], ".del");
}

void getMacroFilename(char *filename, char *macroname) {
    strcpy(filename, _fileBasename);
    strcat(filename, ".m.");
    strcat(filename, macroname);
}

void addFileDeleteList(char *name) {
    agon_fputs(name, FILE_DELETELIST);
    agon_fputs("\n", FILE_DELETELIST);
}

void deleteFiles(void) {
    char line[LINEMAX];
    mos_del(filename[FILE_LOCAL_LABELS]);
    mos_del(filename[FILE_ANONYMOUS_LABELS]);

    // delete all files listed for cleanup
    if(reOpenFile(FILE_DELETELIST, fa_read)) {
        while (agon_fgets(line, sizeof(line), FILE_DELETELIST)){
            trimRight(line);
            if(CLEANUPFILES) mos_del(line);
        }
        mos_fclose(filehandle[FILE_DELETELIST]);
    }
    mos_del(filename[FILE_DELETELIST]);
}

void closeAllFiles() {
    if(filehandle[FILE_CURRENT]) mos_fclose(filehandle[FILE_CURRENT]);
    if(filehandle[FILE_INPUT]) mos_fclose(filehandle[FILE_INPUT]);
    if(filehandle[FILE_OUTPUT]) mos_fclose(filehandle[FILE_OUTPUT]);
    if(filehandle[FILE_LOCAL_LABELS]) mos_fclose(filehandle[FILE_LOCAL_LABELS]);
    if(filehandle[FILE_ANONYMOUS_LABELS]) mos_fclose(filehandle[FILE_ANONYMOUS_LABELS]);
    if(list_enabled && filehandle[FILE_LISTING]) mos_fclose(filehandle[FILE_LISTING]);
    if(filehandle[FILE_MACRO]) mos_fclose(filehandle[FILE_MACRO]);

    deleteFiles();
}

bool openfiles(void) {
    bool status = true;

    status = status && openFile(&filehandle[FILE_DELETELIST], filename[FILE_DELETELIST], fa_write | fa_create_always);
    status = status && openFile(&filehandle[FILE_INPUT], filename[FILE_INPUT], fa_read);
    status = status && openFile(&filehandle[FILE_OUTPUT], filename[FILE_OUTPUT], fa_write | fa_create_always);
    status = status && openFile(&filehandle[FILE_LOCAL_LABELS], filename[FILE_LOCAL_LABELS], fa_write | fa_create_always);
    status = status && openFile(&filehandle[FILE_ANONYMOUS_LABELS], filename[FILE_ANONYMOUS_LABELS], fa_write | fa_create_always);
    if(list_enabled) status = status && openFile(&filehandle[FILE_LISTING], filename[FILE_LISTING], fa_write | fa_create_always);
    if(!status) closeAllFiles();
    return status;
}

// Get a maximum of 'maxsize' characters from a file
char *agon_fgets(char *s, int maxsize, uint8_t fileid) {
	int c;
	char *cs;
	bool eof;
    c = 0;
	cs = s;

    #ifdef AGON // Agon FatFS handles feof differently than C/C++ std library feof
    eof = 0;
	do {
		c = mos_fgetc(filehandle[fileid]);
		if((*cs++ = c) == '\n') break;		
		eof = mos_feof(filehandle[fileid]);
	}
	while(--maxsize > 0 && !eof);
    #endif

    #ifndef AGON
	do {
		c = mos_fgetc(filehandle[fileid]);
		eof = mos_feof(filehandle[fileid]);
		if((*cs++ = c) == '\n') break;		
	}
	while(--maxsize > 0 && !eof);
    #endif

	*cs = '\0';

	return (eof) ? NULL : s;
}

int agon_fputs(char *s, uint8_t fileid) {
    int number = 0;
    while(*s) {
        mos_fputc(filehandle[fileid], *s);
        number++;
        s++;
    }
    return number;
}

void outputBufferedWrite(unsigned char s) {
    _outputbuffer[_outputbuffersize++] = s;
    if(_outputbuffersize == OUTPUT_BUFFERSIZE) outputBufferFlush();
}

void outputBufferInit(void) {
    _outputbuffersize = 0;
}

void outputBufferFlush(void) {
    mos_fwrite(filehandle[FILE_OUTPUT], _outputbuffer, _outputbuffersize);
    _outputbuffersize = 0;
}
