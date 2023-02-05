#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "instruction.h"
#include "label.h"

#define LINEMAX    256
#define START_ADDRESS 0x40000

bool assemble(FILE *infile, FILE *outfile);
void argcheck(bool passed);
void extcheck(bool passed);
void instsize(uint8_t instruction_size);

uint32_t immediate(char *arg);
uint8_t getADLsuffix(void);  // checks defined types and also checks if the allowed type is present vs the adlmode
void emit_ld_from_immediate(uint8_t prefix, uint8_t opcode, char *valstring);
void emit_8bit(uint8_t value);
void emit_16bit(uint16_t value);
void emit_24bit(uint32_t value);

#endif // ASSEMBLE_H