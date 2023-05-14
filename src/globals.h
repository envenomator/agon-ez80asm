#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include "./stdint.h"
#include "instruction.h"
#include "config.h"
#include "macro.h"

typedef struct {
    instruction *current_instruction;
    macro *current_macro;
    char *next;
    char label[32];
    char mnemonic[16];
    char suffix[16];
    char operand1[32];
    char operand2[32];
    char comment[128];
    bool suffixpresent;
    uint16_t size;      // byte size of the assembler-command output in db/defb/dw/defw
} tokenline;

// Global variables
extern bool lineNumberNeedsReset;
extern unsigned int linenumber;
extern unsigned int pass;
extern bool recordingMacro;
extern macro *currentExpandedMacro;
extern uint24_t address;
extern uint24_t totalsize;
extern uint16_t global_errors;
extern bool adlmode;
extern tokenline currentline;
extern bool list_enabled;
extern bool consolelist_enabled;

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
    ERROR_MACRODEFINED,
    ERROR_MACRONAME,
    ERROR_MACROARGCOUNT,
    ERROR_MACROFILEWRITE,
    ERROR_MACROINCORRECTARG,
    ERROR_CHARCONSTANT
};
// Error messages
extern char *message[];

#endif // GLOBALS_H