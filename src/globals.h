#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "instruction.h"
#include "config.h"
#include "macro.h"
#include "assemble.h"

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

typedef enum {
    CONDITIONSTATE_NORMAL,
    CONDITIONSTATE_FALSE,
    CONDITIONSTATE_TRUE
} conditionalstate;

// Global variables
extern uint8_t errorreportlevel;
extern uint8_t maxstackdepth;
extern struct contentitem *currentcontentitem;
extern uint16_t sourcefilecount;
extern uint16_t binfilecount;
extern uint24_t filecontentsize;
extern bool completefilebuffering;
extern unsigned int macrolinenumber;
extern unsigned int pass;
extern conditionalstate inConditionalSection;
extern macro_t *currentExpandedMacro;
extern uint8_t macrolevel;
extern uint24_t address;
extern uint16_t errorcount;
extern bool adlmode;
extern tokenline_t currentline;
extern bool list_enabled;
extern bool consolelist_enabled;
extern uint8_t fillbyte;
extern uint24_t start_address;
extern bool coloroutput;
extern unsigned int labelcollisions;
extern bool ignore_truncation_warnings;
extern bool issue_warning;
extern uint24_t remaining_dsspaces;
extern bool exportsymbols, displaystatistics;

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
    ERROR_ZEROORNEGATIVE,
    ERROR_UNKNOWNLABEL,
    ERROR_INVALIDLABELORNUMBER,
    ERROR_INVALIDLABEL,
    ERROR_LABELTOOLONG,
    ERROR_MISSINGOPERAND,
    ERROR_MISSINGLABELORNUMBER,
    ERROR_INVALIDMNEMONIC,
    ERROR_INVALIDOPERAND,
    ERROR_OPERANDSNOTMATCHING,
    ERROR_TRANSFORMATION,
    WARNING_N_TOOLARGE,
    ERROR_INVALIDSUFFIX,
    WARNING_TRUNCATED_8BIT,
    WARNING_TRUNCATED_16BIT,
    WARNING_TRUNCATED_24BIT,
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
    ERROR_RECURSIVEINCLUDE,
    ERROR_MACRODEFINED,
    ERROR_MACROUNFINISHED,
    ERROR_MACRONOTSTARTED,
    ERROR_MACRONAME,
    ERROR_MACRONAMELENGTH,
    ERROR_MACROARGLENGTH,
    ERROR_MACROARGCOUNT,
    ERROR_MACROINCORRECTARG,
    ERROR_MACROARGSUBSTLENGTH,
    ERROR_MACRO_NOGLOBALLABELS,
    ERROR_MACRO_NOANONYMOUSLABELS,
    ERROR_MACROMEMORYALLOCATION,
    ERROR_MACROINMACRO,
    ERROR_MACROCALLEDINMACRO,
    ERROR_MACROTOOLARGE,
    ERROR_MACROMAXLEVEL,
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
    ERROR_RESETINPUTFILE,
    ERROR_READINGBINFILE,
    ERROR_READINGINPUT,
    ERROR_MEMORY,
    WARNING_UNSUPPORTED_INITIALIZER,
    ERROR_BRACKETFORMAT
};
// Error messages
extern char *message[];

#endif // GLOBALS_H
