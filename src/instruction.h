#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "config.h"
#include "typedefs.h"
#include "hash.h"
#include "utils.h"
#include "macro.h"

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

instruction_t * instruction_lookup(const char *name);
void initInstructionTable(void);
void emit_instruction(const operandlist_t *list);
uint8_t get_immediate_size(uint8_t suffix);

extern instruction_t *instruction_table[INSTRUCTION_HASHTABLESIZE];

#endif // INSTRUCTION_H
