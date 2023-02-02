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

char* rtable[] = {
    "b",
    "c",
    "d",
    "e",
    "h",
    "l",
    "m",
    "a",
    NULL
};

char* irtable[] = {
    "ixh",
    "ixl",
    "iyh",
    "iyl",
    NULL
};

char *rptable[] = {
    "bc",
    "de",
    "hl",
    "sp",
    NULL
};

char *rp2table[] = {
    "bc",
    "de",
    "hl",
    "af",
    NULL
};

char *cctable[] = {
    "nz",
    "z",
    "nc",
    "c",
    "po",
    "pe",
    "p",
    "m",
    NULL
};

uint8_t table_find(char **table, char *search){
    uint8_t i;

    for(i = 0; *table != NULL; i++, table++) {
        if(strcmp(*table, search) == 0) break;
    }
    return i; // caller decides if this is valid or not, according to a specific TABLE_MAX
}

instruction * instruction_table[INSTRUCTION_TABLE_SIZE];
unsigned int collisions;

operandlist operands_adc[] = {
    {OP_A, OP_INDIRECT_HL,  0x00, 0x00, 0x8E, SL_ONLY},
    {OP_A, OP_IXY,          0x00, 0xDD, 0x8C, NONE},
    {OP_A, OP_IXY,          0x00, 0xDD, 0x8D, NONE},
    {OP_A, OP_IXY,          0x00, 0xFD, 0x8C, NONE},
    {OP_A, OP_IXY,          0x00, 0xFD, 0x8D, NONE},
    {OP_A, OP_INDIRECT_IXYd,0x00, 0xDD, 0x8E, SL_ONLY},
    {OP_A, OP_INDIRECT_IXYd,0x00, 0xFD, 0x8E, SL_ONLY},
    {OP_A, OP_N,            0x00, 0x00, 0xCE, NONE},
    {OP_A, OP_R,            0x00, 0x00, 0x88, NONE},
    {OP_HL, OP_RR,          0x00, 0xED, 0x4A, SL_ONLY},
    {OP_HL, OP_SP,          0x00, 0xED, 0x7A, SL_ONLY}
};

operandlist operands_add[] = {
    {OP_A, OP_INDIRECT_HL,  0x00, 0x00, 0x86, SL_ONLY}
};

instruction instructions[] = {
    {"adc", EZ80, operands_adc},
    {"add", EZ80, operands_add},
    {"adl", ASSEMBLER, NULL}
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