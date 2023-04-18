#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "./stdint.h"
#include "mos-interface.h"
#include "io.h"
#include "listing.h"
#include "filestack.h"

// Global variables
char     filename[FILES][FILENAMEMAXLENGTH];
uint8_t  filehandle[FILES];
// Local variables
char *   _bufferstart[FILES];          // statically set start of buffer to each file
char *   _filebuffer[FILES];            // actual moving pointers in buffer
uint24_t _filebuffersize[FILES];        // current fill size of each buffer
bool     _fileEOF[FILES];
char     _inputbuffer[FILE_BUFFERSIZE];
char     _outputbuffer[FILE_BUFFERSIZE];
char     _fileBasename[FILENAMEMAXLENGTH]; // base filename for all output files

void _initFileBufferLayout(void) {
    int n;

    for(n = 0; n < FILES; n++) _bufferstart[n] = 0;
    // static layout
    _bufferstart[FILE_INPUT] = _inputbuffer;
    _bufferstart[FILE_OUTPUT] = _outputbuffer;
}

void _initFileBuffers(void) {
    int n;
    // dynamic layout
    for(n = 0; n < FILES; n++) {
        _filebuffer[n] = _bufferstart[n];
        _filebuffersize[n] = 0;
        _fileEOF[n] = false;
    }
}

// opens a file a places the result at the file pointer
bool _openFile(uint8_t *file, char *name, uint8_t mode) {
    *file = mos_fopen(name, mode);
    if(*file) return true;
    return false;
}

bool reOpenFile(uint8_t number, uint8_t mode) {
    bool result;
    //printf("Re-opening    id: %d\r\n",filehandle[number]);
    if(filehandle[number]) mos_fclose(filehandle[number]);
    result = _openFile(&filehandle[number], filename[number], mode);
    //printf("Re-opened mos id: %d\r\n",filehandle[number]);
    return result;
}

void _prepare_filenames(char *input_filename) {
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

void io_getMacroFilename(char *filename, char *macroname) {
    strcpy(filename, _fileBasename);
    strcat(filename, ".m.");
    strcat(filename, macroname);
}

void io_addDeleteList(char *name) {
    io_puts(FILE_DELETELIST, name);
    io_puts(FILE_DELETELIST, "\n");
}

void _deleteFiles(void) {
    char line[LINEMAX];
    mos_del(filename[FILE_LOCAL_LABELS]);
    mos_del(filename[FILE_ANONYMOUS_LABELS]);

    // delete all files listed for cleanup
    if(reOpenFile(FILE_DELETELIST, fa_read)) {
        while(io_gets(FILE_DELETELIST, line, sizeof(line))) {
            trimRight(line);
            if(CLEANUPFILES) mos_del(line);
        }
        mos_fclose(filehandle[FILE_DELETELIST]);
    }
    mos_del(filename[FILE_DELETELIST]);
}

void _closeAllFiles() {
    if(filehandle[FILE_CURRENT]) mos_fclose(filehandle[FILE_CURRENT]);
    if(filehandle[FILE_INPUT]) mos_fclose(filehandle[FILE_INPUT]);
    if(filehandle[FILE_OUTPUT]) mos_fclose(filehandle[FILE_OUTPUT]);
    if(filehandle[FILE_LOCAL_LABELS]) mos_fclose(filehandle[FILE_LOCAL_LABELS]);
    if(filehandle[FILE_ANONYMOUS_LABELS]) mos_fclose(filehandle[FILE_ANONYMOUS_LABELS]);
    if(list_enabled && filehandle[FILE_LISTING]) mos_fclose(filehandle[FILE_LISTING]);
    if(filehandle[FILE_MACRO]) mos_fclose(filehandle[FILE_MACRO]);
}

bool _openfiles(void) {
    bool status = true;

    status = status && _openFile(&filehandle[FILE_DELETELIST], filename[FILE_DELETELIST], fa_write | fa_create_always);
    status = status && _openFile(&filehandle[FILE_INPUT], filename[FILE_INPUT], fa_read);
    status = status && _openFile(&filehandle[FILE_OUTPUT], filename[FILE_OUTPUT], fa_write | fa_create_always);
    status = status && _openFile(&filehandle[FILE_LOCAL_LABELS], filename[FILE_LOCAL_LABELS], fa_write | fa_create_always);
    status = status && _openFile(&filehandle[FILE_ANONYMOUS_LABELS], filename[FILE_ANONYMOUS_LABELS], fa_write | fa_create_always);
    if(list_enabled) status = status && _openFile(&filehandle[FILE_LISTING], filename[FILE_LISTING], fa_write | fa_create_always);
    if(!status) _closeAllFiles();
    return status;
}

// Will be called for output files only
// These files will have a buffer set up previously
void _io_flush(uint8_t fh) {
    mos_fwrite(filehandle[fh], (char*)_bufferstart[fh], _filebuffersize[fh]);
    _filebuffer[fh] = _bufferstart[fh];
    _filebuffersize[fh] = 0;
}

// Will be called for reading INPUT buffer
void _io_fillbuffer(uint8_t fh) {
    if(_bufferstart[fh]) {
        _filebuffersize[fh] = mos_fread(filehandle[fh], _bufferstart[fh], FILE_BUFFERSIZE);
        _filebuffer[fh] = _bufferstart[fh];
    }
}

// Flush all output files
void _io_flushOutput(void) {
    _io_flush(FILE_OUTPUT);
    _io_flush(FILE_LISTING);
}

// Only called on output-mode files
void io_putc(uint8_t fh, unsigned char c) {
    if(_bufferstart[fh]) {
        // Buffered IO
        *(_filebuffer[fh]++) = c;
        _filebuffersize[fh]++;
        if(_filebuffersize[fh] == FILE_BUFFERSIZE) _io_flush(fh);
    }
    else mos_fputc(filehandle[fh], c); // regular non-buffered IO
}

int io_puts(uint8_t fh, char *s) {
    int number = 0;
    while(*s) {
        io_putc(fh, *s);
        number++;
        s++;
    }
    return number;
}

// Get a maximum of 'size' characters from a file, ends at CR or EOF
char* io_gets(uint8_t fh, char *s, int size) {
	int c;
	char *cs;
    bool eof;
    c = 0;
	cs = s;

    if(_bufferstart[fh]) {
        //printf("new mode read\r\n");
        do { // io_getc
            #ifndef AGON
            if(_filebuffersize[fh] == 0) {
                _io_fillbuffer(fh);
                if(_filebuffersize[fh] == 0) {
                    _fileEOF[fh] = true;
                    return 0;
                }
            }
            #endif
            #ifdef AGON
            if(_filebuffersize[fh] == 0) {
                _fileEOF[fh] = mos_feof(filehandle[fh]);
                if(_fileEOF[fh]) return 0;
                _io_fillbuffer(fh);
            }
            #endif
            _filebuffersize[fh]--;
            c = *(_filebuffer[fh]);
            _filebuffer[fh]++;
            if((*cs++ = c) == '\n') break;
        }
        while(--size > 0 && !_fileEOF[fh]);
        eof = _fileEOF[fh];
    }
    else {
        #ifdef AGON // Agon FatFS handles feof differently than C/C++ std library feof
        eof = 0;
        do {
            c = mos_fgetc(filehandle[fh]);
            if((*cs++ = c) == '\n') break;		
            eof = mos_feof(filehandle[fh]);
        }
        while(--size > 0 && !eof);
        #endif

        #ifndef AGON
        do {
            c = mos_fgetc(filehandle[fh]);
            eof = mos_feof(filehandle[fh]);
            if((*cs++ = c) == '\n') break;		
        }
        while(--size > 0 && !eof);
        #endif
    }
	*cs = '\0';
	return (eof) ? NULL : s;
}

bool io_init(char *input_filename) {
    _prepare_filenames(input_filename);
    _initFileBufferLayout();
    _initFileBuffers();
    return _openfiles();
}

bool io_setpass(uint8_t pass) {
    bool result = true;
    switch(pass) {
        case 1:
            return true;
            break;
        case 2:
            _initFileBuffers();
            result = result && reOpenFile(FILE_INPUT, fa_read);
            result = result && reOpenFile(FILE_LOCAL_LABELS, fa_read);
            result = result && reOpenFile(FILE_ANONYMOUS_LABELS, fa_read);
            if(!result) error("Error re-opening input file\r\n");
            return result;
            break;
    }
    return false;
}

void io_close(void) {
    _io_flushOutput();
    _closeAllFiles();
    _deleteFiles();
}

void io_getCurrent(filestackitem *fsi) {
    fsi->fp = filehandle[FILE_CURRENT];
    fsi->filebuffer = _filebuffer[FILE_CURRENT];
    fsi->bufferstart = _bufferstart[FILE_CURRENT];
    fsi->filebuffersize = _filebuffersize[FILE_CURRENT];
    strcpy(fsi->filename, filename[FILE_CURRENT]);
    fsi->linenumber = linenumber;
    fsi->fileEOF = _fileEOF[FILE_CURRENT];
}

void io_setCurrent(filestackitem *fsi) {
    filehandle[FILE_CURRENT] = fsi->fp;
    _filebuffer[FILE_CURRENT] = fsi->filebuffer;
    _bufferstart[FILE_CURRENT] = fsi->bufferstart;
    _filebuffersize[FILE_CURRENT] = fsi->filebuffersize;
    strcpy(filename[FILE_CURRENT], fsi->filename);
    linenumber = fsi->linenumber;
    _fileEOF[FILE_CURRENT] = fsi->fileEOF;
} 

void io_resetCurrentInput(void) {
    _bufferstart[FILE_CURRENT] = _bufferstart[FILE_INPUT];

    strcpy(filename[FILE_CURRENT], filename[FILE_INPUT]);
    filehandle[FILE_CURRENT] = filehandle[FILE_INPUT];
    _filebuffer[FILE_CURRENT] = _filebuffer[FILE_INPUT];
    _filebuffersize[FILE_CURRENT] = _filebuffersize[FILE_INPUT];
    _fileEOF[FILE_CURRENT] = false;    
}

void io_getFileDefaults(filestackitem *fsi) {
    fsi->filename[0] = 0;
    fsi->fp = 0;
    fsi->filebuffer = 0;
    fsi->filebuffersize = 0;
    fsi->bufferstart = 0;
    fsi->fileEOF = 0;
    fsi->linenumber = 1;
}