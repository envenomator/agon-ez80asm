#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdbool.h>
#include <stdint.h>

#define INSTRUCTION_TABLE_SIZE 256
#define MAX_MNEMONIC_SIZE 6

typedef enum { // permitted operand type
    OP_NONE,
    OP_CC,
    OP_IR,
    OP_IXY,
    OP_INDIRECT_IXYd,
    OP_MMN,
    OP_INDIRECT_MMN,
    OP_N,
    OP_A,
    OP_HL,
    OP_INDIRECT_HL,
    OP_RR,
    OP_INDIRECT_RR,
    OP_RXY,
    OP_SP,
    OP_INDIRECT_SP,
    OP_R,
    OP_REG_R,
    OP_MB,
    OP_I,
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

typedef struct {
    cpuregister reg;
    bool        indirect;
    uint8_t     displacement;
    bool        immediate_provided;
    uint32_t    immediate;
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

typedef struct {
    operandtype operandA;           // Filter for operandA - which register applies?
    operandtype operandB;           // Filter for operandB
    bool        transformA;         // Do we transform acc to operandA
    bool        transformB;         //  "        "       " "  operandB
    uint8_t     prefix1;            // base prefix1, may be transformed by A/B
    uint8_t     prefix2;            // base prefix2, may be transformed by A/B
    uint8_t     opcode;             // base opcode, may be transformed by A/B
    adltype     adl;                // the adl mode allowed in set of operands
} operandlist;

// An array-based index of this structure will act as a fast lookup table
typedef struct {
    operandtype type;
    bool (*match)(operand *);
    void (*transform)(operandlist *, operand *);       // transform output according to given operand
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
