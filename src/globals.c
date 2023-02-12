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
uint32_t totalsize;      // total size of assembled binary
uint16_t global_errors;
bool adlmode;

char outputbuffer[65535];
char * outputbufferptr;

tokenline currentline;
operand operand1;
operand operand2;
char buffer[DBBUFFERSIZE];
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
    "Range error in immediate",
    "Invalid suffix",
    "Value too large for 16-bits, truncation required",
    "Illegal suffix for this mnemonic",
    "Displacement range error",
    "String format error",
    "Invalid ADL mode",
    "Invalid assembler command",
    "New address lower than PC",
    "Address outside 16-bit range",
    "String format error",
    "Relative jump too large",
    "Invalid bit number (0-7)",
    "Illegal interrupt mode (0-2)",
    "Illegal restart address",
    "Value format error",
    "Missing label"
};
