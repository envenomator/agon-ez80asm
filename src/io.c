#include "io.h"
#include "instruction.h"

// File basename variable
char filebasename[FILENAMEMAXLENGTH + 1];

// Global variables
char     filename[FILES][FILENAMEMAXLENGTH + 1];
FILE*    filehandle[FILES];

// Local variables
char *   _bufferstart[FILES];          // statically set start of buffer to each file
char *   _filebuffer[FILES];            // actual moving pointers in buffer
uint24_t _filebuffersize[FILES];        // current fill size of each buffer
bool     _fileEOF[FILES];
char     _outputbuffer[FILE_BUFFERSIZE];

#ifdef CEDEV
    // platform-specific for Agon CEDEV
    int remove(const char *filename) {
        return removefile(filename);
    }
#endif // else use standard remove()

FILE *io_openfile(char *name, char *mode) {
    char buffer[256];
    FILE *fh = fopen(name, mode);
    if(!fh) {
        snprintf(buffer, 256, "Error opening \"%s\"", name);
        error(buffer);
    }
    return fh;
}

uint24_t io_getfilesize(FILE *fh) {
    uint24_t filesize;

    #ifdef CEDEV
        // Use optimal assembly routine in moscalls.asm
        filesize = getfilesize(fh->fhandle);
    #else
        char _buffer[FILE_BUFFERSIZE];
        uint24_t size;
        filesize = 0;
        while(1) {
            // Other non-agon compilers
            size = fread(_buffer, 1, FILE_BUFFERSIZE, fh);
            filesize += size;
            if(size < FILE_BUFFERSIZE) break;
        }
        fseek(fh, 0, SEEK_SET);
    #endif

    return filesize;
}

void _initFileBufferLayout(void) {
    int n;

    for(n = 0; n < FILES; n++) _bufferstart[n] = 0;
    // static layout
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

void create_filebasename(char *input_filename) {
    strcpy(filebasename, input_filename);
    remove_ext(filebasename, '.', '/');
}

// Prepare filenames according to input filename
// If output_filename is given, adopt that, 
// otherwise append base inputfilename + .bin
void _prepare_filenames(char *output_filename) {
    // prepare filenames
    if((output_filename == NULL) || (strlen(output_filename) == 0)) {
        strcpy(filename[FILE_OUTPUT], filebasename);
        strcat(filename[FILE_OUTPUT], ".bin");
    }
    else {
        strcpy(filename[FILE_OUTPUT], output_filename);
    }

    strcpy(filename[FILE_ANONYMOUS_LABELS], filebasename);
    strcpy(filename[FILE_LISTING], filebasename);
    strcat(filename[FILE_ANONYMOUS_LABELS], ".anonlbls");
    strcat(filename[FILE_LISTING], ".lst");
}

void _deleteFiles(void) {
    if(CLEANUPFILES) {
        remove(filename[FILE_ANONYMOUS_LABELS]);
    }
    if(global_errors && CLEANUPFILES) remove(filename[FILE_OUTPUT]);
}

void _closeAllFiles() {
    if(filehandle[FILE_OUTPUT]) fclose(filehandle[FILE_OUTPUT]);
    if(filehandle[FILE_ANONYMOUS_LABELS]) fclose(filehandle[FILE_ANONYMOUS_LABELS]);
    if(list_enabled && filehandle[FILE_LISTING]) fclose(filehandle[FILE_LISTING]);
}

bool _openfiles(void) {
    bool status = true;

    status = status && _openFile(FILE_OUTPUT, "wb+");
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

bool io_init(char *input_filename, char *output_filename) {
    sourcefilecount = 0;
    binfilecount = 0;
    create_filebasename(input_filename);
    _prepare_filenames(output_filename);
    _initFileBufferLayout();
    _initFileBuffers();
    //_init_labelscope();
    return _openfiles();
}

void io_close(void) {
    _io_flushOutput();
    _closeAllFiles();
    _deleteFiles();
}

void emit_8bit(uint8_t value) {
    if(pass == 2) {
        if(list_enabled || consolelist_enabled) listEmit8bit(value);
        io_outputc(value);
    }
    address++;
}

void emit_16bit(uint16_t value) {
    emit_8bit(value&0xFF);
    emit_8bit((value>>8)&0xFF);
}

void emit_24bit(uint24_t value) {
    emit_8bit(value&0xFF);
    emit_8bit((value>>8)&0xFF);
    emit_8bit((value>>16)&0xFF);
}

void emit_32bit(uint32_t value) {
    emit_8bit(value&0xFF);
    emit_8bit((value>>8)&0xFF);
    emit_8bit((value>>16)&0xFF);
    emit_8bit((value>>24)&0xFF);
}

void emit_adlsuffix_code(uint8_t suffix) {
    uint8_t code;
    switch(suffix) {
        case S_SIS:
            code = CODE_SIS;
            break;
        case S_LIS:
            code = CODE_LIS;
            break;
        case S_SIL:
            code = CODE_SIL;
            break;
        case S_LIL:
            code = CODE_LIL;
            break;
        default:
            error(message[ERROR_INVALIDSUFFIX]);
            return;
    }
    emit_8bit(code);
}

// emits a string surrounded by literal string quotes, as the token gets in from a file
// Only called when the first character is a double quote
void emit_quotedstring(char *str) {
    bool escaped = false;
    uint8_t escaped_char;

    str++; // skip past first "
    while(*str) {
        if(!escaped) {
            if(*str == '\\') { // escape character
                escaped = true;
            }
            else {
                if(*str == '\"') return;
                else emit_8bit(*str);
            }
        }
        else { // previously escaped
            escaped_char = getEscapedChar(*str);
            if(escaped_char == 0xff) {
                error(message[ERROR_ILLEGAL_ESCAPESEQUENCE]);
                return;
            }
            emit_8bit(escaped_char);
            escaped = false;
        }
        str++;
    }
    // we missed an end-quote to this string, we shouldn't reach this
    error(message[ERROR_STRING_NOTTERMINATED]);
}

// Emit a 16 or 24 bit immediate number, according to
// given suffix bit, or in lack of it, the current ADL mode
void emit_immediate(operand_t *op, uint8_t suffix) {
    uint8_t num;

    num = get_immediate_size(suffix);
    emit_8bit(op->immediate & 0xFF);
    emit_8bit((op->immediate >> 8) & 0xFF);
    if(num == 3) emit_8bit((op->immediate >> 16) & 0xFF);
}
