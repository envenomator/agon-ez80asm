#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "hash.h"
#include "utils.h"
#include "macro.h"

#define MAX_MNEMONIC_SIZE         32

// Individual registers - 24-bit bitfield
#define R_NONE  0x000000
#define R_A     0x000001
#define R_B     0x000002
#define R_C     0x000004
#define R_D     0x000008
#define R_E     0x000010
#define R_H     0x000020
#define R_L     0x000040
#define R_BC    0x000080
#define R_DE    0x000100
#define R_HL    0x000200
#define R_SP    0x000400
#define R_AF    0x000800
#define R_IX    0x001000
#define R_IY    0x002000
#define R_IXH   0x004000
#define R_IXL   0x008000
#define R_IYH   0x010000
#define R_IYL   0x020000
#define R_R     0x040000
#define R_MB    0x080000
#define R_I     0x100000

// Register sets (mask filters)
#define RS_NONE    0x0
#define RS_R    R_A | R_B | R_C | R_D | R_E | R_H | R_L
#define RS_RR   R_BC | R_DE | R_HL
#define RS_IR   R_IXH | R_IXL | R_IYH | R_IYL
#define RS_IXY  R_IX | R_IY
#define RS_RXY  R_BC | R_DE | R_IX | R_IY
#define RS_XY   R_IX | R_IY
#define RS_AE   R_A | R_B | R_C | R_D | R_E

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

// Status bitfield codes
#define NOREQ         0x00    // no requirement
#define INDIRECT      0x01    // bit 1              - checked for match during processInstructions
#define IMM           0x02    // bit 2              - checked for match during processInstructions
#define CC            0x04    // bit 3              - checked for match during processInstructions
#define CCA           0x08    // bit 4              - checked for match during processInstructions
#define IMM_N         0x10    // bit 5              - immediate length information for emission
#define IMM_MMN       0x20    // bit 6              - immediate length information for emission
#define IMM_BIT       0x40    // bit 7              - immediate length information for emission
#define IMM_NSELECT   0x80    // bit 8              - immediate length information for emission
#define MODECHECK     (INDIRECT | IMM | CC | CCA)

// Flag bitfield codes
#define F_NONE          0x00    // Nothing set
#define F_DISPA         0x01    // Displacement required for operand A
#define F_DISPB         0x02    // Displacement required for operand B
#define F_CCOK          0x04    // Condition code accepted as operand
#define F_DDFDOK        0x08    // DD/FD accepted
#define S_SIS           0x10    // SUFFIX permitted
#define S_LIS           0x20    // SUFFIX permitted
#define S_SIL           0x40    // SUFFIX permitted
#define S_LIL           0x80    // SUFFIX permitted
#define S_ANY           S_SIS | S_LIS | S_SIL | S_LIL
#define S_SISLIL        S_SIS | S_LIL
#define S_S1L0          S_SIL | S_LIS
#define S_LILLIS        S_LIL | S_LIS

// actual codes to emit when permitted
#define CODE_SIS    0x40
#define CODE_LIS    0x49
#define CODE_SIL    0x52
#define CODE_LIL    0x5B

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

typedef struct {
    char       *name;
    uint8_t     type;                       // EZ80 / Assembler / MACRO
    uint8_t     asmtype;                    // assembler subcommand
    uint8_t     listnumber;                 // number of items to iterate over in the list
    operandlist_t *list;
    macro_t    *macro;
    void       *next;
} instruction_t;

instruction_t * instruction_lookup(char *name);
void initInstructionTable(void);
void emit_instruction(operandlist_t *list);
uint8_t get_immediate_size(uint8_t suffix);

extern instruction_t *instruction_table[INSTRUCTION_HASHTABLESIZE];

#endif // INSTRUCTION_H
