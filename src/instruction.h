#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <stdbool.h>

#define ENDMACROCMD "endmacro"
#define MAX_MNEMONIC_SIZE         10

typedef enum { // permitted operand type
    OPTYPE_NONE,
    OPTYPE_CC,
    OPTYPE_IR,
    OPTYPE_IXY,
    OPTYPE_MMN,
    OPTYPE_INDIRECT_MMN,
    OPTYPE_N,
    OPTYPE_A,
    OPTYPE_HL,
    OPTYPE_INDIRECT_HL,
    OPTYPE_RR,
    OPTYPE_INDIRECT_RR,
    OPTYPE_RXY,
    OPTYPE_SP,
    OPTYPE_INDIRECT_SP,
    OPTYPE_R,
    OPTYPE_REG_R,
    OPTYPE_MB,
    OPTYPE_I,
    OPTYPE_BIT,
    OPTYPE_AF,
    OPTYPE_DE,
    OPTYPE_NSELECT,
    OPTYPE_INDIRECT_N,
    OPTYPE_INDIRECT_BC,
    OPTYPE_INDIRECT_C,
    OPTYPE_INDIRECT_IXY,
    OPTYPE_CCA,
    OPTYPE_INDIRECT_DE,
    OPTYPE_IX,
    OPTYPE_IY,
    OPTYPE_R_AEONLY,
    OPTYPE_IXYd,                // From here on, only DISPLACEMENT OPTYPES for fast access
    OPTYPE_INDIRECT_IXYd,       // Displacement type
    OPTYPE_IXd,                 // Displacement type
    OPTYPE_IYd,                 // Displacement type
    OPTYPE_INDIRECT_IXd,        // Displacement type
    OPTYPE_INDIRECT_IYd         // Displacement type
} permittype_t;

typedef enum {
    R_NONE,
    R_A,
    R_B,
    R_C,
    R_D,
    R_E,
    R_H,
    R_L,
    R_BC,
    R_DE,
    R_HL,
    R_SP,
    R_AF,
    R_IX,
    R_IY,
    R_IXH,
    R_IXL,
    R_IYH,
    R_IYL,
    R_R,
    R_MB,
    R_I
} cpuregister;

#define R_INDEX_B   0
#define R_INDEX_C   1
#define R_INDEX_D   2
#define R_INDEX_E   3
#define R_INDEX_H   4
#define R_INDEX_L   5
#define R_INDEX_M   6
#define R_INDEX_A   7

#define R_INDEX_BC  0
#define R_INDEX_DE  1
#define R_INDEX_HL  2
#define R_INDEX_SP  3
#define R_INDEX_AF  3
#define R_INDEX_IX  2   // same as HL
#define R_INDEX_IY  2   // same as HL

#define R_INDEX_I   0
#define R_INDEX_MB  0
#define R_INDEX_R   0

#define CC_INDEX_NZ 0
#define CC_INDEX_Z  1
#define CC_INDEX_NC 2
#define CC_INDEX_C  3
#define CC_INDEX_PO 4
#define CC_INDEX_PE 5
#define CC_INDEX_P  6
#define CC_INDEX_M  7

// bitfield codes to check allowed emission
#define S_SIS         0x01
#define S_LIS         0x02
#define S_SIL         0x04
#define S_LIL         0x08
#define S_ANY         0xFF
#define S_NONE        0x00
#define S_SISLIL      S_SIS | S_LIL
#define S_S1L0        S_SIL | S_LIS

// actual codes to emit
#define CODE_SIS    0x40
#define CODE_LIS    0x49
#define CODE_SIL    0x52
#define CODE_LIL    0x5B

typedef struct {
    cpuregister         reg;
    uint8_t             reg_index;
    bool                indirect;
    bool                cc;
    uint8_t             cc_index;
    int16_t             displacement;           // larger, so we can check range
    bool                displacement_provided;
    bool                immediate_provided;
    int32_t             immediate;
} operand_t;

typedef struct {
    char                name[MAX_MNEMONIC_SIZE];
    cpuregister         reg;
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

typedef enum {
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
}opcodetransformtype_t;

typedef struct {
    permittype_t          operandA;           // Filter for operandA - which register applies?
    permittype_t          operandB;           // Filter for operandB
    bool                  ddfdpermitted;         
    opcodetransformtype_t transformA;         // Do we transform acc to operandA
    opcodetransformtype_t transformB;         //  "        "       " "  operandB
    uint8_t               prefix;            // base prefix1, or 0 if none to output
    uint8_t               opcode;             // base opcode, may be transformed by A/B, according to opcodetransformtype
    uint8_t               adl;                // the adl mode allowed in set of operands
} operandlist_t;

enum {
    EZ80,
    ASSEMBLER
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

typedef enum {
    ASM_ARG_NONE,
    ASM_ARG_SINGLE,
    ASM_ARG_LIST,
    ASM_ARG_KEYVAL
} asm_argtype;

typedef struct {
    char        name[MAX_MNEMONIC_SIZE];
    uint8_t     type;                       // EZ80 / Assembler
    uint8_t     asmtype;                    // assembler subcommand
    uint8_t     listnumber;                 // number of items to iterate over in the list
    operandlist_t *list;
    asm_argtype asmargument;
} instruction_t;

instruction_t * instruction_table_lookup(char *name);
//regcc_t * regcc_table_lookup(char *key);
instruction_t * instruction_hashtable_lookup(char *name);

//void setup_regcc_hashtable(void);
void init_instruction_hashtable(void);


// An array-based index of this structure will act as a fast lookup table
typedef struct {
    permittype_t type;
    bool (*match)(operand_t *);
} permittype_match_t;

extern permittype_match_t permittype_matchlist[];
#endif // INSTRUCTION_H
