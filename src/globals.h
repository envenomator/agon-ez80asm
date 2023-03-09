#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdint.h>
#include "instruction.h"
#include "config.h"

enum {
    FILE_CURRENT,
    FILE_INPUT,
    FILE_OUTPUT,
    FILE_LOCAL_LABELS,
    FILE_ANONYMOUS_LABELS,
    FILE_LISTING,
    FILE_MACRO
};

typedef struct {
    instruction *current_instruction;
    char *next;
    char label[32];
    char mnemonic[16];
    char suffix[16];
    char operand1[32];
    char operand2[32];
    char comment[128];
    uint16_t size;      // byte size of the assembler-command output in db/defb/dw/defw
} tokenline;


// Global variables
extern bool consolelist_enabled;
extern bool lineNumberNeedsReset;

extern char filename[FILES][FILENAMEMAXLENGTH];    // 0 - current, 1 - input, 2 - binary output, 3 - local labels, 4 - anonymous labels, 5 - listing
extern uint8_t filehandle[FILES];
//extern FILE *file_currentinput;
//extern char currentInputFilename[FILENAMEMAXLENGTH];

extern unsigned int linenumber;
extern unsigned int pass;
extern bool inmacro;
extern uint24_t address;
extern uint24_t totalsize;
extern uint16_t global_errors;
extern bool adlmode;
extern tokenline currentline;

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
    WARNING_N_8BITRANGE,
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
    ERROR_LISTFORMAT,
    ERROR_CREATINGLABEL,
    ERROR_LABELDEFINED,
    ERROR_MAXLOCALLABELS,
    ERROR_INCLUDEFILE,
    ERROR_LOCALLABELSNOTALLOWED,
    ERROR_ASCIIFORMAT,
    ERROR_PARSE,
    ERROR_OPERATOR,
    ERROR_POWER2,
    ERROR_MAXINCLUDEFILES,
    ERROR_MAXMACROS,
    ERROR_MACRODEFINED
};
// Error messages
extern char *message[];

#endif // GLOBALS_H