#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "hash.h"
#include "instruction.h"
#include "assemble.h"
#include "utils.h"
#include "globals.h"

bool none_match(operand *op) {
    return ((op->reg == R_NONE) && (op->immediate_provided == false));
}

bool cc_match(operand *op) {
    return false; // not implemented yet
}

bool ir_match(operand *op) {
    return ((op->reg >= R_IXH) && (op->reg <= R_IYL));
}

bool ixy_match(operand *op) {
    return ((op->reg == R_IX) || (op->reg == R_IY));
}

bool indirect_ixyd_match(operand *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && op->indirect);
}

bool mmn_match(operand *op) {
    return (!(op->indirect) && (op->immediate_provided));
}

bool indirect_mmn_match(operand *op) {
    return ((op->indirect) && (op->immediate_provided));
}

bool n_match(operand *op) {
    return (!(op->indirect) && (op->immediate_provided));
}

bool a_match(operand *op) {
    return(op->reg == R_A);
}

bool hl_match(operand *op) {
    return((op->reg == R_HL) && !(op->indirect));
}

bool indirect_hl_match(operand *op) {
    return((op->reg == R_HL) && (op->indirect));
}

bool rr_match(operand *op) {
    return((op->reg >= R_BC) && (op->reg <= R_HL) && !(op->indirect));
}

bool indirect_rr_match(operand *op) {
    return((op->reg >= R_BC) && (op->reg <= R_HL) && (op->indirect));
}

bool rxy_match(operand *op) {
    return(!(op->indirect) && ((op->reg == R_BC) || (op->reg == R_DE) || (op->reg == R_IX) || (op->reg == R_IY)));
}

bool sp_match(operand *op) {
    return(!(op->indirect) && (op->reg == R_SP));
}

bool indirect_sp_match(operand *op) {
    return((op->indirect) && (op->reg == R_SP));
}

bool r_match(operand *op) {
    return((op->reg >= R_A) && (op->reg <= R_L));
}

bool reg_r_match(operand *op) {
    return(op->reg == R_R);
}

bool mb_match(operand *op) {
    return(op->reg == R_MB);
}

bool i_match(operand *op) {
    return(op->reg == R_I);
}

instruction * instruction_table[INSTRUCTION_TABLE_SIZE]; // hashtable of possible instructions, indexed by mnemonic name
operandtype_match operandtype_matchlist[] = {            // table with fast access to functions that perform matching to an specific operandtype
    {OP_NONE, none_match},
    {OP_CC, cc_match},
    {OP_IR, ir_match},
    {OP_IXY, ixy_match},
    {OP_INDIRECT_IXYd, indirect_ixyd_match},
    {OP_MMN, mmn_match},
    {OP_INDIRECT_MMN, indirect_mmn_match},
    {OP_N, n_match},
    {OP_A, a_match},
    {OP_HL, hl_match},
    {OP_INDIRECT_HL, indirect_hl_match},
    {OP_RR, rr_match},
    {OP_INDIRECT_RR, indirect_rr_match},
    {OP_RXY, rxy_match},
    {OP_SP, sp_match},
    {OP_INDIRECT_SP, indirect_sp_match},
    {OP_R, r_match},
    {OP_REG_R, reg_r_match},
    {OP_MB, mb_match},
    {OP_I, i_match}
};

unsigned int collisions;    // internal use

operandlist operands_adc[] = {
    {OP_A, OP_INDIRECT_HL,  0x00, 0x00, 0x8E, SL_ONLY},
    {OP_A, OP_IR,           0x00, 0xDD, 0x8C, NONE},
    {OP_A, OP_INDIRECT_IXYd,0x00, 0xDD, 0x8E, SL_ONLY},
    {OP_A, OP_N,            0x00, 0x00, 0xCE, NONE},
    {OP_A, OP_R,            0x00, 0x00, 0x88, NONE},
    {OP_HL, OP_RR,          0x00, 0xED, 0x4A, SL_ONLY},
    {OP_HL, OP_SP,          0x00, 0xED, 0x7A, SL_ONLY},
};

operandlist operands_add[] = {
    {OP_A, OP_INDIRECT_HL,  0x00, 0x00, 0x86, SL_ONLY},
};

operandlist operands_test[] = {
    {OP_NONE, OP_NONE, 0x00,0x00,0x80, NONE},
};

instruction instructions[] = {
    {"test",EZ80, sizeof(operands_test)/sizeof(operandlist),operands_test},
    {"adc", EZ80, sizeof(operands_adc)/sizeof(operandlist), operands_adc},
    {"add", EZ80, sizeof(operands_add)/sizeof(operandlist), operands_add},
    {"adl", ASSEMBLER, 0, NULL}
};

/*
mnemonic mnemonics[] = {
    {.name = "adc", .function = zero_action},
    {.name = "add", .function = zero_action},
    {.name = "cp", .function = zero_action},
    {.name = "daa", .function = zero_action},
    {.name = "dec", .function = zero_action},
    {.name = "inc", .function = zero_action},
    {.name = "mlt", .function = zero_action},
    {.name = "neg", .function = zero_action},
    {.name = "sbc", .function = zero_action},
    {.name = "sub", .function = zero_action},
    {.name = "bit", .function = zero_action},
    {.name = "res", .function = zero_action},
    {.name = "set", .function = zero_action},
    {.name = "cpd", .function = zero_action},
    {.name = "cpdr", .function = zero_action},
    {.name = "cpi", .function = zero_action},
    {.name = "cpir", .function = zero_action},
    {.name = "ldd", .function = zero_action},
    {.name = "lddr", .function = zero_action},
    {.name = "ldi", .function = zero_action},
    {.name = "ldir", .function = zero_action},
    {.name = "ex", .function = zero_action},
    {.name = "exx", .function = zero_action},
    {.name = "in", .function = zero_action},
    {.name = "in0", .function = zero_action},
    {.name = "ind", .function = zero_action},
    {.name = "indr", .function = zero_action},
    {.name = "indrx", .function = zero_action},
    {.name = "ind2", .function = zero_action},
    {.name = "ind2r", .function = zero_action},
    {.name = "indm", .function = zero_action},
    {.name = "indmr", .function = zero_action},
    {.name = "ini", .function = zero_action},
    {.name = "inir", .function = zero_action},
    {.name = "inirx", .function = zero_action},
    {.name = "ini2", .function = zero_action},
    {.name = "ini2r", .function = zero_action},
    {.name = "inim", .function = zero_action},
    {.name = "inimr", .function = zero_action},
    {.name = "otdm", .function = zero_action},
    {.name = "otdmr", .function = zero_action},
    {.name = "otdrx", .function = zero_action},
    {.name = "otim", .function = zero_action},
    {.name = "otimr", .function = zero_action},
    {.name = "otirx", .function = zero_action},
    {.name = "out", .function = zero_action},
    {.name = "out0", .function = zero_action},
    {.name = "outd", .function = zero_action},
    {.name = "otdr", .function = zero_action},
    {.name = "outd2", .function = zero_action},
    {.name = "otd2r", .function = zero_action},
    {.name = "outi", .function = zero_action},
    {.name = "otir", .function = zero_action},
    {.name = "outi2", .function = zero_action},
    {.name = "oti2r", .function = zero_action},
    {.name = "tstio", .function = zero_action},
    {.name = "ld", .function = ld_action},
    {.name = "lea", .function = zero_action},
    {.name = "pea", .function = zero_action},
    {.name = "pop", .function = zero_action},
    {.name = "push", .function = zero_action},
    {.name = "and", .function = zero_action},
    {.name = "cpl", .function = zero_action},
    {.name = "or", .function = zero_action},
    {.name = "tst", .function = zero_action},
    {.name = "xor", .function = zero_action},
    {.name = "ccf", .function = zero_action},
    {.name = "di", .function = zero_action},
    {.name = "ei", .function = zero_action},
    {.name = "halt", .function = zero_action},
    {.name = "im", .function = zero_action},
    {.name = "nop", .function = nop_action},
    {.name = "rsmix", .function = zero_action},
    {.name = "scf", .function = zero_action},
    {.name = "slp", .function = zero_action},
    {.name = "stmix", .function = zero_action},
    {.name = "call", .function = zero_action},
    {.name = "djnz", .function = zero_action},
    {.name = "jp", .function = zero_action},
    {.name = "jr", .function = zero_action},
    {.name = "ret", .function = ret_action},
    {.name = "reti", .function = zero_action},
    {.name = "retn", .function = zero_action},
    {.name = "rst", .function = zero_action},
    {.name = "rl", .function = zero_action},
    {.name = "rla", .function = zero_action},
    {.name = "rlc", .function = zero_action},
    {.name = "rlca", .function = zero_action},
    {.name = "rld", .function = zero_action},
    {.name = "rr", .function = zero_action},
    {.name = "rra", .function = zero_action},
    {.name = "rrc", .function = zero_action},
    {.name = "rrca", .function = zero_action},
    {.name = "rrd", .function = zero_action},
    {.name = "sla", .function = zero_action},
    {.name = "sra", .function = zero_action},
    {.name = "srl", .function = zero_action},
    {.name = "adl", .function = adl_action}
};
*/

bool instruction_table_insert(instruction *p){
    int index,i,try;

    if(p == NULL) return false;
    index = hash(p->name, INSTRUCTION_TABLE_SIZE);
   
    for(i = 0; i < INSTRUCTION_TABLE_SIZE; i++) {
        try = (i + index) % INSTRUCTION_TABLE_SIZE;
        if(instruction_table[try] == NULL){
            instruction_table[try] = p;
            return true;
        } 
        else collisions++;
    }
    return false;
}

void init_instruction_table(){
    int i;
    for(i = 0; i < INSTRUCTION_TABLE_SIZE; i++){
        instruction_table[i] = NULL;
    }
    collisions = 0;

    for(i = 0; i < sizeof(instructions)/sizeof(instruction);i++){
        instruction_table_insert(&instructions[i]);
    }
}

unsigned int instruction_table_entries(){
    unsigned int i,count = 0;
    for(i = 0; i < INSTRUCTION_TABLE_SIZE; i++){
        if(instruction_table[i] != NULL) count++;
    }
    return count;
}


void print_instruction_table(){
    int i;
    for(i = 0; i < INSTRUCTION_TABLE_SIZE; i++){
        if(instruction_table[i] == NULL) {
            printf("\t%i\t---\n",i);
        } else {
            printf("\t%i\t%s\n",i,instruction_table[i]->name);
        }
    }
}

instruction * instruction_table_lookup(char *name){
    int index,i,try;
    index = hash(name, INSTRUCTION_TABLE_SIZE);
    for(i = 0; i < INSTRUCTION_TABLE_SIZE; i++){
        try = (index + i) % INSTRUCTION_TABLE_SIZE;
        if(instruction_table[try] == NULL){
            return NULL;
        }
        if(instruction_table[try] != NULL &&
            strncmp(instruction_table[try]->name,name,INSTRUCTION_TABLE_SIZE) == 0){
            return instruction_table[try];
        }
    }
    return NULL;
}