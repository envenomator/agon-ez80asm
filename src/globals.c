#include "globals.h"
#include <stdio.h>
#include <stdint.h>

/*
 * Global variables
 */

bool lineNumberNeedsReset;
unsigned int linenumber;
unsigned int pass;
bool recordingMacro;
int inConditionalSection;
macro_t *currentExpandedMacro;
uint24_t address;
uint16_t global_errors;
bool adlmode;
bool list_enabled;
bool consolelist_enabled;
uint8_t fillbyte;
uint24_t start_address;


tokenline_t currentline;

operand_t operand1;
operand_t operand2;
opcodesequence_t output;

char *message[] = {
    "Invalid register in operand",
    "Missing opening bracket in operand",
    "Missing closing bracket in operand",
    "Invalid number format",
    "Invalid label reference",
    "Invalid label name",
    "Missing operand",
    "Invalid mnemonic",
    "Invalid operand",
    "Operand(s) not matching mnemonic",
    "Error in opcode transformation",
    "Range error in immediate",
    "Invalid 8-bit range",
    "Value too large for 8-bit, truncation required",
    "Invalid suffix",
    "Value too large for 16-bits, truncation required",
    "Suffix not matching mnemonic / ADL mode",
    "Index register offset exceeded",
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
    "Missing label",
    "Wordsize larger than 16-bit while ADL set to 0",
    "Too many arguments",
    "Invalid list format",
    "Error creating label",
    "Label already defined",
    "Maximum number of local labels reached",
    "Unable to open include file",
    "Local label reference not allowed",
    "Invalid literal format",
    "Parse error",
    "Invalid operator",
    "Argument is not a power of 2",
    "Maximum nested level of include files reached",
    "Maximum number of macros reached",
    "Macro already defined",
    "Invalid macro name",
    "Macro maximum name length",
    "Macro maximum arg length",
    "Maximum number of macro arguments reached",
    "Error writing to macro file",
    "Incorrect number of macro arguments",
    "Unsupported character constant",
    "Input line too long",
    "Missing conditional expression",
    "Nested conditionals not supported",
    "Missing IF directive",
    "Missing ENDIF directive",
    "Internal error"
};
