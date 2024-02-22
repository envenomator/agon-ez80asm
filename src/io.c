#include "io.h"

// Global variables
char     filename[FILES][FILENAMEMAXLENGTH + 1];
FILE*    filehandle[FILES];
uint16_t sourcefilecount;
uint16_t binfilecount;

// Local variables
char *   _bufferstart[FILES];          // statically set start of buffer to each file
char *   _filebuffer[FILES];            // actual moving pointers in buffer
uint24_t _filebuffersize[FILES];        // current fill size of each buffer
bool     _fileEOF[FILES];
char     _inputbuffer[FILE_BUFFERSIZE];
char     _outputbuffer[FILE_BUFFERSIZE];
char     _fileBasename[FILENAMEMAXLENGTH + 1]; // base filename for all output files

#ifdef CEDEV
    // platform-specific for Agon CEDEV
    int remove(const char *filename) {
        return removefile(filename);
    }
#endif // else use standard remove()

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
bool _openFile(uint8_t filenumber, char* mode) {
    FILE* file = fopen(filename[filenumber], mode);
    filehandle[filenumber] = file;
    if(file) return true;
    else return false;
}

// Prepare filenames according to input filename
// If output_filename is given, adopt that, 
// otherwise append base inputfilename + .bin
void _prepare_filenames(char *input_filename, char *output_filename) {
    // prepare filenames
    strcpy(filename[FILE_INPUT], input_filename);
    strcpy(filename[FILE_OUTPUT], output_filename);
    strcpy(_fileBasename, input_filename);
    remove_ext(_fileBasename, '.', '/');
    if((strlen(output_filename) == 0) || (output_filename == NULL)){
        strcpy(filename[FILE_OUTPUT], _fileBasename);
        strcat(filename[FILE_OUTPUT], ".bin");
    }
    strcpy(filename[FILE_LOCAL_LABELS], _fileBasename);
    strcpy(filename[FILE_ANONYMOUS_LABELS],_fileBasename);
    strcpy(filename[FILE_SYMBOLS],_fileBasename);
    if(list_enabled) strcpy(filename[FILE_LISTING],_fileBasename);
    strcat(filename[FILE_LOCAL_LABELS], ".lcllbls");
    strcat(filename[FILE_ANONYMOUS_LABELS], ".anonlbls");
    if(list_enabled) strcat(filename[FILE_LISTING], ".lst");
    strcat(filename[FILE_SYMBOLS], ".symbols");
}

void io_getMacroFilename(char *filename, char *macroname) {
    strcpy(filename, _fileBasename);
    strcat(filename, ".m.");
    strcat(filename, macroname);
}

void _deleteFiles(void) {
    char macrofilename[FILENAMEMAXLENGTH];
    int n;

    if(CLEANUPFILES) {
        remove(filename[FILE_LOCAL_LABELS]);
        remove(filename[FILE_ANONYMOUS_LABELS]);
    }
    for(n = 0; n < macroTableCounter; n++) {
        io_getMacroFilename(macrofilename, macroTable[n].name);
        if(CLEANUPFILES) remove(macrofilename);

    }

    if(global_errors && CLEANUPFILES) remove(filename[FILE_OUTPUT]);
}

void _closeAllFiles() {
    if(filehandle[FILE_INPUT]) fclose(filehandle[FILE_INPUT]);
    if(filehandle[FILE_OUTPUT]) fclose(filehandle[FILE_OUTPUT]);
    if(filehandle[FILE_LOCAL_LABELS]) fclose(filehandle[FILE_LOCAL_LABELS]);
    if(filehandle[FILE_ANONYMOUS_LABELS]) fclose(filehandle[FILE_ANONYMOUS_LABELS]);
    if(list_enabled && filehandle[FILE_LISTING]) fclose(filehandle[FILE_LISTING]);
    if(filehandle[FILE_MACRO]) fclose(filehandle[FILE_MACRO]);
}

bool _openfiles(void) {
    bool status = true;

    status = status && _openFile(FILE_INPUT, "rb");
    status = status && _openFile(FILE_OUTPUT, "wb+");
    status = status && _openFile(FILE_LOCAL_LABELS, "wb+");
    status = status && _openFile(FILE_ANONYMOUS_LABELS, "wb+");
    if(list_enabled) status = status && _openFile(FILE_LISTING, "w");
    if(!status) _closeAllFiles();
    return status;
}

// Will be called for output files only
// These files will have a buffer set up previously
void _io_flush(uint8_t fh) {
    fwrite(_bufferstart[fh], 1, _filebuffersize[fh], filehandle[fh]);
    _filebuffer[fh] = _bufferstart[fh];
    _filebuffersize[fh] = 0;
}

// Will be called for reading INPUT buffer
void _io_fillbuffer(uint8_t fh) {
    if(_bufferstart[fh]) {
        _filebuffersize[fh] = fread(_bufferstart[fh], 1, FILE_BUFFERSIZE, filehandle[fh]);
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
    else fputc(c, filehandle[fh]); // regular non-buffered IO
}

void io_outputc(unsigned char c) {
    *(_filebuffer[FILE_OUTPUT]++) = c;
    _filebuffersize[FILE_OUTPUT]++;
    if(_filebuffersize[FILE_OUTPUT] == FILE_BUFFERSIZE) _io_flush(FILE_OUTPUT);
}

void  io_write(uint8_t fh, char *s, uint16_t size) {
    if(_bufferstart[fh]) {
        // Buffered IO
        while(size--) {
            *(_filebuffer[fh]++) = *s++;
            _filebuffersize[fh]++;
            if(_filebuffersize[fh] == FILE_BUFFERSIZE) _io_flush(fh);
        }
    }
    else fwrite(s, 1, size, filehandle[fh]);
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

// Allocate a LINEMAX buffer for io_getline, max LINEMAX-1 chars will be read
// Configured to use LINEMAX = uint8_t Maximum for performance
char* io_getline(uint8_t fh, char *s) {
	uint8_t maxchars, charsread;
    char *cs,*ptr;
    bool finalread = false;
    bool done = false;

    cs = s;
    while(!done && !_fileEOF[fh]) {
        if(_filebuffersize[fh] == 0) {
            if(finalread) _fileEOF[fh] = true;
            else {
                _io_fillbuffer(fh);
                if(_filebuffersize[fh] < FILE_BUFFERSIZE) finalread = true;
            }
        }
        else {
            ptr = _filebuffer[fh]; // pointer to read buffer;
            maxchars = (_filebuffersize[fh] > LINEMAX-1)?LINEMAX-1:(uint8_t)_filebuffersize[fh];
            charsread = 0;
            while(maxchars--) {
                charsread++;
                if(((*cs++ = *ptr++)) == '\n') {
                    done = true;
                    break;
                }
            }
            _filebuffersize[fh] -= charsread;
            _filebuffer[fh] += charsread;
        }
    }
    *cs = '\0';
    return (*s == 0)? NULL:s;
}

bool io_init(char *input_filename, char *output_filename) {
    sourcefilecount = 0;
    binfilecount = 0;
    _prepare_filenames(input_filename, output_filename);
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
            result = result && (fseek(filehandle[FILE_INPUT], 0, 0) == 0);
            result = result && (fseek(filehandle[FILE_LOCAL_LABELS], 0, 0) == 0);
            result = result && (fseek(filehandle[FILE_ANONYMOUS_LABELS], 0, 0) == 0);
            if(!result) error("Error resetting input file(s)\r\n");
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