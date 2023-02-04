#include "globals.h"
#include <stdint.h>
#include <stdbool.h>
/*
 * Global variables
 */
bool debug_enabled;
bool listing_enabled;
FILE *infile;
FILE *outfile;
unsigned int linenumber;
unsigned int pass;
uint32_t address;
uint16_t global_errors;
bool adlmode;

tokenline currentline;
operand operand1;
operand operand2;
opcodesequence output;

/*
 * Messages
 */
char *message[] = {
    "Invalid register in operand",
    "Missing opening bracket in operand",
    "Missing closing bracket in operand",
    "Invalid number format",
    "Invalid label definition",
    "Missing operand",
    "Invalid mnemonic",
    "Invalid operand",
    "Operand(s) not matching mnemonic",
    "Error in opcode transformation",
    "Constant in operand larger than 8 bit"
};
