#include "globals.h"
#include <stdint.h>
#include <stdbool.h>
/*
 * Global variables
 */
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
