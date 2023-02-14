#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "instruction.h"

#define DBBUFFERSIZE 256

typedef struct {
    instruction *current_instruction;
    char *next;
    char label[32];
    char mnemonic[16];
    char suffix[16];
    char operand1[32];
    char operand2[32];
    char comment[128];
    char buffer[DBBUFFERSIZE];
    uint16_t size;      // byte size of the assembler-command output in db/defb/dw/defw
} tokenline;


// Global variables
extern bool debug_enabled;
extern bool listing_enabled;
extern FILE *infile;
extern FILE *outfile;
extern FILE *locals;
extern char localsfilename[256];
extern unsigned int linenumber;
extern unsigned int pass;
extern uint32_t address;
extern uint32_t totalsize;
extern uint16_t global_errors;
extern bool adlmode;
extern tokenline currentline;

extern char outputbuffer[];
char * outputbufferptr;

// Global parsed results
extern uint8_t suffix;      // per-instruction suffix code
extern operand operand1;
extern operand operand2;
extern opcodesequence output;

// Errors
enum {
    ERROR_INVALIDREGISTER,
    ERROR_OPENINGBRACKET,
    ERROR_CLOSINGBRACKET,
    ERROR_INVALIDNUMBER,
    ERROR_INVALIDLABEL,
    ERROR_MISSINGOPERAND,
    ERROR_INVALIDMNEMONIC,
    ERROR_INVALIDOPERAND,
    ERROR_OPERANDSNOTMATCHING,
    ERROR_TRANSFORMATION,
    WARNING_N_TOOLARGE,
    ERROR_INVALIDSUFFIX,
    ERROR_MMN_TOOLARGE,
    ERROR_ILLEGAL_SUFFIXMODE,
    ERROR_DISPLACEMENT_RANGE,
    ERROR_STRING_NOTTERMINATED,
    ERROR_INVALID_ADLMODE,
    ERROR_INVALID_ASSEMBLERCMD,
    ERROR_ADDRESSLOWER,
    ERROR_ADDRESSRANGE,
    ERROR_STRINGFORMAT,
    ERROR_RELATIVEJUMPTOOLARGE,
    ERROR_INVALIDBITNUMBER,
    ERROR_ILLEGALINTERRUPTMODE,
    ERROR_ILLEGALRESTARTADDRESS,
    ERROR_VALUEFORMAT,
    ERROR_MISSINGLABEL,
    ERROR_ADLWORDSIZE,
    ERROR_TOOMANYARGUMENTS,
    ERROR_LISTFORMAT
};
// Error messages
extern char *message[];

#endif // GLOBALS_H