#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "instruction.h"
#include "config.h"
#include "macro.h"

typedef struct {
    instruction_t *current_instruction;
    macro_t *current_macro;
    char *next;
    char *label;
    char *mnemonic;
    char *suffix;
    char *operand1;
    char *operand2;
    char *comment;
    bool suffixpresent;
    uint16_t size;      // byte size of the assembler-command output in db/defb/dw/defw
} tokenline_t;


// Global variables
extern bool lineNumberNeedsReset;
extern unsigned int linenumber;
extern unsigned int pass;
extern bool recordingMacro;
extern int inConditionalSection; // 0: outside conditional section, 1: negative section, 2: positive section
extern macro_t *currentExpandedMacro;
extern uint24_t address;
extern uint16_t global_errors;
extern bool adlmode;
extern tokenline_t currentline;
extern bool list_enabled;
extern bool consolelist_enabled;
extern uint8_t fillbyte;
extern uint24_t start_address;
extern bool coloroutput;

extern unsigned int labelcollisions;
// Global parsed results
extern uint8_t suffix;      // per-instruction suffix code
extern operand_t operand1;
extern operand_t operand2;
extern opcodesequence_t output;


// Errors
enum {
    ERROR_OPENINGBRACKET,
    ERROR_CLOSINGBRACKET,
    ERROR_INVALIDNUMBER,
    ERROR_UNKNOWNLABEL,
    ERROR_INVALIDLABELORNUMBER,
    ERROR_INVALIDLABEL,
    ERROR_MISSINGOPERAND,
    ERROR_INVALIDMNEMONIC,
    ERROR_INVALIDOPERAND,
    ERROR_OPERANDSNOTMATCHING,
    ERROR_TRANSFORMATION,
    WARNING_N_TOOLARGE,
    ERROR_8BITRANGE,
    ERROR_16BITRANGE,
    ERROR_24BITRANGE,
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
    ERROR_UNARYOPERATOR,
    ERROR_POWER2,
    ERROR_MAXINCLUDEFILES,
    ERROR_MAXMACROS,
    ERROR_MACRODEFINED,
    ERROR_MACRONAME,
    ERROR_MACRONAMELENGTH,
    ERROR_MACROARGLENGTH,
    ERROR_MACROARGCOUNT,
    ERROR_MACROFILEWRITE,
    ERROR_MACROINCORRECTARG,
    ERROR_MACRO_NOGLOBALLABELS,
    ERROR_WRITINGMACROFILE,
    ERROR_ILLEGAL_ESCAPELITERAL,
    ERROR_LINETOOLONG,
    ERROR_CONDITIONALEXPRESSION,
    ERROR_NESTEDCONDITIONALS,
    ERROR_MISSINGIFCONDITION,
    ERROR_MISSINGENDIF,
    ERROR_INTERNAL,
    ERROR_ILLEGAL_ESCAPESEQUENCE,
    ERROR_STRING_NOTALLOWED,
    ERROR_FILEGLOBALLABELS,
    ERROR_RESETINPUTFILE
};
// Error messages
extern char *message[];

#endif // GLOBALS_H
