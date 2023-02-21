#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "instruction.h"
#include "label.h"

#define START_ADDRESS   0x0
#define FILLBYTE        0x00 // NOP

bool assemble(FILE *infile, char *filename);
void emit_8bit(uint8_t value);
void emit_16bit(uint16_t value);
void emit_24bit(uint32_t value);

enum {
    STATE_LINESTART,
    STATE_MNEMONIC,
    STATE_SUFFIX,
    STATE_OPERAND1,
    STATE_OPERAND2,
    STATE_ASM_ARG,
    STATE_COMMENT,
    STATE_DONE,
    STATE_MISSINGOPERAND,
    STATE_ASM_STRINGARG,
    STATE_ASM_STRINGARG_CLOSEOUT,
    STATE_ASM_VALUE_ENTRY,
    STATE_ASM_VALUE_EXIT,
    STATE_ASM_VALUE,
    STATE_ASM_PARSE,
    STATE_ASM_VALUE_CLOSEOUT,
};


typedef enum {
    PS_START,
    PS_LABEL,
    PS_COMMAND,
    PS_OP1,
    PS_OP2,
    PS_COMMENT,
    PS_DONE,
    PS_ERROR
} parsestate;

#endif // ASSEMBLE_H