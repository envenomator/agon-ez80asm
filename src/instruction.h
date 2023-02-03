#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdbool.h>
#include <stdint.h>

#define INSTRUCTION_TABLE_SIZE 256
#define MAX_MNEMONIC_SIZE 6

typedef enum {
    OP_EMPTY,
    OP_CC,
    OP_CC_ACCENT,
    OP_D,
    OP_IR,
    OP_IXY,
    OP_INDIRECT_IXY,
    OP_INDIRECT_IXYd,
    OP_MMN,
    OP_INDIRECT_MMN,
    OP_N,
    OP_A,
    OP_R,
    OP_HL,
    OP_INDIRECT_HL,
    OP_RR,
    OP_INDIRECT_RR,
    OP_RXY,
    OP_S,
    OP_SP,
    OP_INDIRECT_SP,
    OP_SS,
    OP_OTHER
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
    operandtype operandA;
    operandtype operandB;
    uint8_t prefix1;
    uint8_t prefix2;
    uint8_t opcode;
    adltype adl;
} operandlist;

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

#define R_INVALID 255

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
