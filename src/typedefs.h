#ifndef TYPEDEF_H
#define TYPEDEF_H

#include "config.h"
#include <stdbool.h>

typedef struct contentitem {
    // Static items
    char         *name;                         // name of the file
    unsigned int  size;                         // size of the file
    char         *buffer;                       // pointer to 1) full file content OR 2) partial content during minimal buffering
    FILE         *fh;                           // filehandle
    void         *next;
    // Items changed during processing
    char         *readptr;
    uint16_t      currentlinenumber;
    char          labelscope[MAXNAMELENGTH+1];
    uint8_t       inConditionalSection;
    unsigned int  bytesinbuffer;                // only used during minimal input buffering
} contentitem_t;

typedef struct {
    uint24_t            reg;
    uint8_t             reg_index;
    bool                indirect;
    bool                cc;
    uint8_t             cc_index;
    int16_t             displacement;           // larger, so we can check range
    bool                displacement_provided;
    bool                immediate_provided;
    int32_t             immediate;
    uint8_t             addressmode;
    char                immediate_name[LINEMAX+1];
    // No new members after previous array: the array isn't FULLY cleared every init of operand_t, only the first byte is set to 0
} operand_t;

typedef struct {
    char                name[MAX_MNEMONIC_SIZE];
    uint24_t            reg;
    uint8_t             reg_index;
    bool                cc;
    uint8_t             cc_index;
} regcc_t;

typedef struct {
    uint8_t suffix;
    uint8_t prefix1;
    uint8_t prefix2;
    uint8_t opcode;
} opcodesequence_t;

typedef struct {
    uint24_t              regsetA;            // one or more registers that need to match this operand
    uint8_t               conditionsA;        // specific addressing conditions that need to match this operand
    uint24_t              regsetB;
    uint8_t               conditionsB;
    uint8_t               transformA;         // Do we transform acc to operandA
    uint8_t               transformB;         //  "        "       " "  operandB
    uint8_t               flags;
    uint8_t               prefix;             // base prefix1, or 0 if none to output
    uint8_t               opcode;             // base opcode, may be transformed by A/B, according to opcodetransformtype
} operandlist_t;

typedef struct {
    char*   name;
    char*   originfilename;
   uint24_t originlinenumber;
    char*   body;
    uint8_t argcount;
    char**  arguments;
    char**  substitutions;
    void*   next;
   uint24_t currentExpandID;
} macro_t;

typedef struct {
    char       *name;
    uint8_t     type;                       // EZ80 / Assembler / MACRO
    uint8_t     asmtype;                    // assembler subcommand
    uint8_t     listnumber;                 // number of items to iterate over in the list
    operandlist_t *list;
    macro_t    *macro;
    void       *next;
} instruction_t;

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
} conditionalstate_t;

typedef struct {
    char *name;
    bool  local;
    void *next;
    uint24_t address;
} label_t;

typedef struct {
    uint8_t scope;
    bool defined;
    uint24_t address;
} anonymouslabel_t;

typedef enum {
    PS_START,
    PS_LABEL,
    PS_COMMAND,
    PS_OP,
    PS_COMMENT,
} parsestate_t;

enum {
    TRANSFORM_NONE,
    TRANSFORM_X,
    TRANSFORM_Y,
    TRANSFORM_Z,
    TRANSFORM_P,
    TRANSFORM_Q,
    TRANSFORM_DDFD,
    TRANSFORM_CC,
    TRANSFORM_IR0,
    TRANSFORM_IR3,
    TRANSFORM_SELECT,
    TRANSFORM_N,            // only used by RST
    TRANSFORM_BIT,          // only used by RES/SET
    TRANSFORM_REL,          // only used by JR/DJNZ
};

enum {
    EZ80,
    ASSEMBLER,
    MACRO
};

enum {
    ASM_ALIGN,
    ASM_ADL,
    ASM_ORG,
    ASM_DB,
    ASM_DS,
    ASM_DW,
    ASM_DW24,
    ASM_DW32,
    ASM_ASCIZ,
    ASM_EQU,
    ASM_INCLUDE,
    ASM_BLKB,
    ASM_BLKW,
    ASM_BLKP,
    ASM_BLKL,
    ASM_MACRO_START,
    ASM_MACRO_END,
    ASM_INCBIN,
    ASM_FILLBYTE,
    ASM_IF,
    ASM_ELSE,
    ASM_ENDIF
};

enum {
    FILE_OUTPUT,
    FILE_ANONYMOUS_LABELS,
    FILE_LISTING,
};

enum {
    LABEL_REGULAR,
    LABEL_MACRO
};

// requiredResult
typedef enum {
    REQUIRED_FIRSTPASS,
    REQUIRED_LASTPASS
} requiredResult_t;

// DEFAULT COLOR INDEXES
enum {
    DARK_BLACK = 0,
    DARK_RED,
    DARK_GREEN,
    DARK_YELLOW,
    DARK_BLUE,
    DARK_MAGENTA,
    DARK_CYAN,
    DARK_WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE
};

// ERROR LEVELS
enum {
    LEVEL_ERROR,
    LEVEL_WARNING
};

// Token type that points to the (changed) underlying data string
typedef struct {
    char    *start;
    char    *next;
    char    terminator;
} streamtoken_t;

typedef enum {
    TOKEN_REGULAR,
    TOKEN_STRING,
    TOKEN_LITERAL,
    TOKEN_BRACKET
} tokenclass_t;

typedef enum {
    START,
    OP,
    UNARY,
    NUMBER,
} getValueState_t;

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
    ERROR_MACROARGNAME,
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

#endif