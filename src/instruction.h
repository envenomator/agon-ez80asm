#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdbool.h>
#include <stdint.h>

#define INSTRUCTION_TABLE_SIZE 256
#define MAX_MNEMONIC_SIZE 6

typedef enum { // permitted operand type
    OPTYPE_NONE,
    OPTYPE_CC,
    OPTYPE_IR,
    OPTYPE_IXY,
    OPTYPE_IXYd,
    OPTYPE_INDIRECT_IXYd,
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
    OPTYPE_INDIRECT_IXY
} operandtype;

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


typedef enum {
    POS_SOURCE,
    POS_DESTINATION
} operand_position;

typedef struct {
    operand_position    position;
    cpuregister         reg;
    uint8_t             reg_index;
    bool                indirect;
    bool                cc;
    uint8_t             cc_index;
    uint8_t             displacement;
    bool                displacement_provided;
    bool                immediate_provided;
    uint32_t            immediate;
} operand;

typedef struct {
    uint8_t prefix1;
    uint8_t prefix2;
    uint8_t opcode;
} opcodesequence;

typedef enum {
    NONE,
    ANY,
    L_ONLY,
    SL_ONLY,
    SISLIL
} adltype;

typedef enum {
    TRANSFORM_NONE,
    TRANSFORM_X,
    TRANSFORM_Y,
    TRANSFORM_Z,
    TRANSFORM_P,
    TRANSFORM_Q,
    TRANSFORM_DDFD
}opcodetransformtype;

typedef struct {
    operandtype         operandA;           // Filter for operandA - which register applies?
    operandtype         operandB;           // Filter for operandB
    opcodetransformtype transformA;         // Do we transform acc to operandA
    opcodetransformtype transformB;         //  "        "       " "  operandB
    uint8_t             prefix;            // base prefix1, may be transformed by A/B
    uint8_t             opcode;             // base opcode, may be transformed by A/B
    adltype             adl;                // the adl mode allowed in set of operands
} operandlist;

// An array-based index of this structure will act as a fast lookup table
typedef struct {
    operandtype type;
    bool (*match)(operand *);
    void (*transform)(opcodetransformtype type, operand *);       // transform output according to given operand
} operandtype_match;

enum {
    EZ80,
    ASSEMBLER
};

typedef struct {
    char        name[MAX_MNEMONIC_SIZE];
    uint8_t     type;
    uint8_t     listnumber;
    operandlist *list;
} instruction;

void init_instruction_table();
instruction * instruction_table_lookup(char *name);

extern operandtype_match operandtype_matchlist[];
#endif // INSTRUCTION_H
