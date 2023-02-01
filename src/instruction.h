#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdbool.h>
#include <stdint.h>

#define INSTRUCTION_TABLE_SIZE 256
#define MAX_MNEMONIC_SIZE 6

typedef enum {
    OP_R,
    OP_RR,
    OP_HL,
    OP_SP,
    OP_RR_I,
    OP_HL_I,
    OP_IX,
    OP_IY,
    OP_IXPLUSD_I, // (ix/y+d)
    OP_IYPLUSD_I, // (ix/y+d)
    OP_MMN,
    OP_MMN_I,
    OP_SP_I,
    OP_A,
    OP_I,
    OP_IXH, // ir
    OP_IXL, // ir
    OP_IYH, // ir
    OP_IYL, // ir
    OP_MB,
    OP_N,
    OP_REFRESH
} operandtype;

typedef enum {
    NONE,
    ANY,
    L_ONLY,
    SL_ONLY,
    SISLIL
} adltype;


typedef struct {
    operandtype type;
    uint8_t     reg;
    bool        indirect;
    uint8_t     displacement;
    uint32_t    immediate;
} operand;

typedef struct {
    uint8_t operandA;
    uint8_t operandB;
    uint8_t prefix1;
    uint8_t prefix2;
    uint8_t opcode;
    uint8_t immediate;
    adltype adl;
} operandlist;

enum {
    EZ80,
    ASSEMBLER
};

typedef struct {
    char        name[MAX_MNEMONIC_SIZE];
    uint8_t     type;
    operandlist *o;
} instruction;

#define R_B 0
#define R_C 1
#define R_D 2
#define R_E 3
#define R_H 4
#define R_L 5
#define R_M 6
#define R_A 7
#define RTABLE_MAX 8

#define R_I 10
#define R_R 11

#define IRTABLE_MAX 4
#define RPTABLE_MAX 4
#define RR_BC 0
#define RR_DE 1
#define RR_HL 2
#define RR_SP 3
#define RR_AF 3

#define RR_IX 10
#define RR_IY 11
#define RR_IXH 12
#define RR_IXL 13
#define RR_IYH 14
#define RR_IYL 15
#define RR_MB 16
#define CCTABLE_MAX 8

void init_instruction_table();
instruction * instruction_table_lookup(char *name);

#endif // INSTRUCTION_H
