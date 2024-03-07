#include "instruction.h"
#include "moscalls.h"
#include "globals.h"
#include "label.h"
#include "io.h"

// instruction hash table
instruction_t *instruction_hashtable[INSTRUCTION_HASHTABLESIZE];

// get the number of bytes to emit from an immediate
uint8_t get_immediate_size(uint8_t suffix) {
    if(suffix) {
        if(suffix & (S_SIS | S_LIS)) return 2;
        if(suffix & (S_SIL | S_LIL)) return 3;
        error(message[ERROR_INVALIDMNEMONIC]);
        return 0;
    }
    if(adlmode) return 3;
    else return 2;
}

uint8_t get_ddfd_prefix(uint24_t reg) {
    if(reg & (R_IX | R_IXH | R_IXL)) return 0xDD;
    if(reg & (R_IY | R_IYH | R_IYL)) return 0xFD;
    return 0;
}

void prefix_ddfd_suffix(operandlist_t *op) {
    uint8_t prefix1, prefix2;

    if(!op->ddfdpermitted) return;

    prefix1 = get_ddfd_prefix(operand1.reg);
    prefix2 = get_ddfd_prefix(operand2.reg);

    if((!prefix1 && prefix2) || (!operand1.indirect && prefix1 && prefix2)) {
        output.prefix1 = prefix2;
    }
    else {
        output.prefix1 = prefix1;
    }
}

void transform_instruction(operand_t *op, uint8_t type) {
    uint8_t y;
    int24_t rel;

    switch(type) {
        case TRANSFORM_IR0:
            if(op->reg & (R_IXL | R_IYL)) output.opcode |= 0x01;
            break;
        case TRANSFORM_IR3:
            if(op->reg & (R_IXL | R_IYL)) output.opcode |= 0x08;
            break;
        case TRANSFORM_Z:
            output.opcode |= op->reg_index;
            break;
        case TRANSFORM_Y:
            if(op->immediate_provided) output.opcode |= (op->immediate << 3);
            else output.opcode |= (op->reg_index << 3);
            break;
        case TRANSFORM_P:
            output.opcode |= (op->reg_index << 4);
            break;
        case TRANSFORM_CC:
            output.opcode |= (op->cc_index << 3);
            break;
        case TRANSFORM_N:
            output.opcode |= op->immediate;
            op->immediate_provided = false; // no separate output for this transform
            break;
        case TRANSFORM_BIT:
            output.opcode |= (op->immediate << 3);
            op->immediate_provided = false;
            break;
        case TRANSFORM_SELECT:
            switch(op->immediate) {
                case 0:
                    y = 0;
                    break;
                case 1:
                    y = 2;
                    break;
                case 2:
                    y = 3;
                    break;
                default:
                    y = 0;
            }
            output.opcode |= (y << 3); // shift 3 lower bits 3 to the left
            op->immediate_provided = false; // no separate output for this transform
            break;
        case TRANSFORM_REL:
            if(pass == 2) {
                // label still potentially unknown in pass 1, so output the existing '0' in pass 1
                rel = op->immediate - address - 2;
                if((rel > 127) || (rel < -128)) {
                    error(message[ERROR_RELATIVEJUMPTOOLARGE]);
                }
                op->immediate = ((int8_t)(rel & 0xFF));
                op->immediate_provided = true;
            }
            break;
        case TRANSFORM_NONE:
            break;
        default:
            error(message[ERROR_TRANSFORMATION]);
            break;
    }
    return;
}

// return ADL prefix bitfield, or 0 if none present
uint8_t getADLsuffix(void) {

    if(currentline.suffixpresent == false) return 0;

    switch(strlen(currentline.suffix)) {
        case 1: // .s or .l
            switch(tolower(currentline.suffix[0])) {
                case 's':
                    if(adlmode) return S_SIL;  // SIL
                    else return S_SIS;         // SIS
                    break;
                case 'l':
                    if(adlmode) return S_LIL;  // LIL
                    else return S_LIS;         // LIS
                    break;
                default: // illegal suffix
                    break;
            }
            break;
        case 2: // .is or .il
            if(tolower(currentline.suffix[0]) != 'i') break; // illegal suffix
            switch(tolower(currentline.suffix[1])) {
                case 's':
                    if(adlmode) return S_LIS;  // LIS
                    else return S_SIS;         // SIS
                    break;
                case 'l':
                    if(adlmode) return S_LIL;  // LIL
                    else return S_SIL;         // SIL
                    break;
                default: // illegal suffix
                    break;
            }
            break;
        case 3:
            if(tolower(currentline.suffix[1]) != 'i') break; // illegal suffix
            switch(tolower(currentline.suffix[0])) {
                case 's':
                    if(tolower(currentline.suffix[2]) == 's') return S_SIS; // SIS
                    if(tolower(currentline.suffix[2]) == 'l') return S_SIL; // SIL
                    // illegal suffix
                    break;
                case 'l':
                    if(tolower(currentline.suffix[2]) == 's') return S_LIS; // LIS
                    if(tolower(currentline.suffix[2]) == 'l') return S_LIL; // LIL
                    // illegal suffix
                    break;
                default: // illegal suffix
                    break;
            }
            break;
        default: // illegal suffix
            break;
    }
    error(message[ERROR_INVALIDSUFFIX]);
    return 0;
}

void emit_instruction(operandlist_t *list) {
    bool ddbeforeopcode; // determine position of displacement byte in case of DDCBdd/DDFDdd
    
    // Transform necessary prefix/opcode in output, according to given list and operands
    output.suffix = getADLsuffix();
    output.prefix1 = 0;
    output.prefix2 = list->prefix;
    output.opcode = list->opcode;

    definelabel(address);

    // issue any errors here
    if((list->transformA != TRANSFORM_REL) && (list->transformB != TRANSFORM_REL)) { // TRANSFORM_REL will mask to 0xFF
        if((list->conditionsA & IMM_N) && ((operand1.immediate > 0xFF) || (operand1.immediate < -128))) error(message[ERROR_8BITRANGE]);
        if((list->conditionsB & IMM_N) && ((operand2.immediate > 0xFF) || (operand2.immediate < -128))) error(message[ERROR_8BITRANGE]);
    }
    if((output.suffix) && ((list->adl & output.suffix) == 0)) error(message[ERROR_ILLEGAL_SUFFIXMODE]);
    if((list->displacement_requiredB) && ((operand2.displacement < -128) || (operand2.displacement > 127))) error(message[ERROR_DISPLACEMENT_RANGE]);

    // Specific checks
    if((list->conditionsA & IMM_BIT) && (operand1.immediate > 7)) error(message[ERROR_INVALIDBITNUMBER]);
    if((list->conditionsA & IMM_NSELECT) && (operand1.immediate > 2)) error(message[ERROR_ILLEGALINTERRUPTMODE]);
    if((list->transformA == TRANSFORM_N) && (operand1.immediate & 0x47)) error(message[ERROR_ILLEGALRESTARTADDRESS]);

    // prepare extra DD/FD suffix if needed
    prefix_ddfd_suffix(list);
    // Transform the opcode and potential immediate values, according to the current ruleset
    transform_instruction(&operand1, (uint8_t)list->transformA);
    transform_instruction(&operand2, (uint8_t)list->transformB);
    // determine position of dd
    ddbeforeopcode = (((output.prefix1 == 0xDD) || (output.prefix1 == 0xFD)) && (output.prefix2 == 0xCB) &&
                ((list->displacement_requiredA) || (list->displacement_requiredB)));
    
    // output adl suffix and any prefixes
    if(output.suffix) emit_adlsuffix_code(output.suffix);
    if(output.prefix1) emit_8bit(output.prefix1);
    if(output.prefix2) emit_8bit(output.prefix2);

    // opcode in normal position
    if(!ddbeforeopcode) emit_8bit(output.opcode);
    
    // output displacement
    if(list->displacement_requiredA) emit_8bit(operand1.displacement);
    if(list->displacement_requiredB) emit_8bit(operand2.displacement);
    
    // output n
    if((operand1.immediate_provided) && (list->conditionsA & IMM_N)) emit_8bit(operand1.immediate);
    if((operand2.immediate_provided) && (list->conditionsB & IMM_N)) emit_8bit(operand2.immediate);

    // opcode in DDCBdd/DFCBdd position
    if(ddbeforeopcode) emit_8bit(output.opcode);

    //output remaining immediate bytes
    if(list->conditionsA & IMM_MMN) emit_immediate(&operand1, output.suffix);
    if(list->conditionsB & IMM_MMN) emit_immediate(&operand2, output.suffix);
}
operandlist_t operands_adc[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x8E},
   {R_A,NOREQ,RS_IR,NOREQ,TRANSFORM_NONE,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0x8C},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x8E},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xCE},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0x88},
// same set, without A register
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x8E},
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR0,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0x8C},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x8E},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xCE},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x88},

   {R_HL,NOREQ,RS_RR,NOREQ,TRANSFORM_NONE,TRANSFORM_P,false,false,false,false,S_ANY,0xED,0x4A},
   {R_HL,NOREQ,R_SP,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x7A},
};
operandlist_t operands_add[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
// optimized set
   {R_HL,NOREQ,RS_RR,NOREQ,TRANSFORM_NONE,TRANSFORM_P,false,false,false,false,S_ANY,0x00,0x09},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0x80},
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x86},
// end optimized set
   {R_A,NOREQ,RS_IR,NOREQ,TRANSFORM_NONE,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0x84},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x86},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xC6},
// same set, without A register
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x86},
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR0,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0x84},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x86},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xC6},

   {R_HL,NOREQ,R_SP,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x39},
   {RS_IXY,NOREQ,RS_RXY,NOREQ,TRANSFORM_NONE,TRANSFORM_P,false,false,false,true,S_ANY,0x00,0x09},
   {RS_IXY,NOREQ,R_SP,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0x39},
};
operandlist_t operands_and[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
// optimized set
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xA0},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xE6},
// end optimized set
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xA6},
   {R_A,NOREQ,RS_IR,NOREQ,TRANSFORM_NONE,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0xA4},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0xA6},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xE6},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0xA0},
// same set, without A register
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xA6},
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR0,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0xA4},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0xA6},
};
operandlist_t operands_bit[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,IMMEDIATE | IMM_BIT,R_HL,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x46},
   {RS_NONE,IMMEDIATE | IMM_BIT,RS_IXY,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,true,false,true,S_ANY,0xCB,0x46},
   {RS_NONE,IMMEDIATE | IMM_BIT,RS_R,NOREQ,TRANSFORM_Y,TRANSFORM_Z,false,false,false,false,S_NONE,0xCB,0x40},
};
operandlist_t operands_call[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,IMMEDIATE | IMM_MMN,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xCD},
   {RS_NONE,NOREQ,RS_NONE,IMMEDIATE | IMM_MMN,TRANSFORM_CC,TRANSFORM_NONE,false,false,true,false,S_ANY,0x00,0xC4},
};
operandlist_t operands_ccf[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x3F},
};
operandlist_t operands_cp[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
// optimized set
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xFE},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xB8},
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xBE},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xFE},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0xB8},
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xBE},
// end optimized set
   {R_A,NOREQ,RS_IR,NOREQ,TRANSFORM_NONE,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0xBC},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0xBE},
// same set, without A register
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR0,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0xBC},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0xBE},
};
operandlist_t operands_cpd[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xA9},
};
operandlist_t operands_cpdr[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xB9},
};
operandlist_t operands_cpi[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xA1},
};
operandlist_t operands_cpir[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xB1},
};
operandlist_t operands_cpl[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x2F},
};
operandlist_t operands_daa[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x27},
};
operandlist_t operands_dec[]= {
// optimized set
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_RR,NOREQ,RS_NONE,NOREQ,TRANSFORM_P,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x0B},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x05},
// end optimized set
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x35},
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR3,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0x25},
   {RS_IXY,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0x2B},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x35},
   {R_SP,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x3B},
};
operandlist_t operands_di[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xF3},
};
operandlist_t operands_djnz[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_REL,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x10},
};
operandlist_t operands_ei[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xFB},
};
operandlist_t operands_ex[]= {
// optimized set
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_DE,NOREQ,R_HL,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xEB},
// end optimized set
   {R_AF,NOREQ,R_AF,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x08},
   {R_SP,INDIRECT,R_HL,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xE3},
   {R_SP,INDIRECT,RS_IXY,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0xE3},
};
operandlist_t operands_exx[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xD9},
};
operandlist_t operands_halt[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x76},
};
operandlist_t operands_im[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,IMMEDIATE | IMM_NSELECT,RS_NONE,NOREQ,TRANSFORM_SELECT,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x46},
};
operandlist_t operands_in[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_A,NOREQ,RS_NONE,INDIRECT_IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xDB},
   {RS_R,NOREQ,R_BC,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x40},
   {RS_R,NOREQ,R_C,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x40},
};
operandlist_t operands_in0[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_R,NOREQ,RS_NONE,INDIRECT_IMMEDIATE | IMM_N,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x00},
};
operandlist_t operands_inc[]= {
// optimized set
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_RR,NOREQ,RS_NONE,NOREQ,TRANSFORM_P,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x03},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x04},
   {RS_IXY,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0x23},
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x34},
// end optimized set
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR3,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0x24},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x34},
   {R_SP,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x33},
};
operandlist_t operands_ind[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xAA},
};
operandlist_t operands_ind2[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x8C},
};
operandlist_t operands_ind2r[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x9C},
};
operandlist_t operands_indm[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x8A},
};
operandlist_t operands_indmr[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x9A},
};
operandlist_t operands_indr[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xBA},
};
operandlist_t operands_indrx[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xCA},
};
operandlist_t operands_ini[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xA2},
};
operandlist_t operands_ini2[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x84},
};
operandlist_t operands_ini2r[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x94},
};
operandlist_t operands_inim[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x82},
};
operandlist_t operands_inimr[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x92},
};
operandlist_t operands_inir[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xB2},
};
operandlist_t operands_inirx[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xC2},
};
operandlist_t operands_jp[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,CC,RS_NONE,IMMEDIATE | IMM_MMN,TRANSFORM_CC,TRANSFORM_NONE,false,false,true,false,S_SISLIL,0x00,0xC2},
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xE9},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_SISLIL,0x00,0xE9},
   {RS_NONE,IMMEDIATE | IMM_MMN,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_SISLIL,0x00,0xC3},
};
operandlist_t operands_jr[]= {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,CCA,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_CC,TRANSFORM_REL,false,false,true,false,S_NONE,0x00,0x20},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_REL,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x18},
};
operandlist_t operands_ld[] = {
// start optimized set
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_RR,NOREQ,RS_NONE,IMMEDIATE | IMM_MMN,TRANSFORM_P,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0x01},
   {RS_R,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x06},
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x7E},
   {RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,R_A,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x32},
   {RS_R,NOREQ,RS_R,NOREQ,TRANSFORM_Y,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0x40},
   {R_HL,NOREQ,RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x2A},
   {R_A,NOREQ,RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x3A},
   {RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,R_HL,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x22},
   {R_HL,INDIRECT,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_ANY,0x00,0x70},
   {R_A,NOREQ,R_DE,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x1A},
   {RS_RR,NOREQ,R_HL,INDIRECT,TRANSFORM_P,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x07},
   {RS_RR,INDIRECT,R_A,NOREQ,TRANSFORM_P,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x02},
   {RS_IXY,NOREQ,RS_NONE,IMMEDIATE | IMM_MMN,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0x21},
   {RS_IXY,INDIRECT,RS_RR,NOREQ,TRANSFORM_NONE,TRANSFORM_P,true,false,false,true,S_ANY,0x00,0x0F},
   {RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,RS_IXY,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0x22},
// end optimized set
   {R_A,NOREQ,R_I,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x57},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x7E},
   {R_A,NOREQ,R_MB,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x6E},
   {R_A,NOREQ,R_R,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x5F},
   {R_A,NOREQ,R_BC,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x0A},
   {R_HL,NOREQ,R_I,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0xD7},
   {R_HL,INDIRECT,R_IX,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x3F},
   {R_HL,INDIRECT,R_IY,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x3E},
   {R_HL,INDIRECT,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x36},
   {R_HL,INDIRECT,RS_RR,NOREQ,TRANSFORM_NONE,TRANSFORM_P,false,false,false,false,S_ANY,0xED,0x0F},
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_I,NOREQ,R_HL,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0xC7},
   {R_I,NOREQ,R_A,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x47},
   {RS_IR,NOREQ,RS_IR,NOREQ,TRANSFORM_IR3,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0x64},
   {RS_IR,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_IR3,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0x26},
   {RS_IR,NOREQ,RS_AE,NOREQ,TRANSFORM_IR3,TRANSFORM_Z,false,false,false,true,S_NONE,0x00,0x60},
   {R_IX,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x37},
   {R_IY,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x31},
   {R_IX,NOREQ,R_IX,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x37},
   {R_IY,NOREQ,R_IY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x37},
   {R_IX,NOREQ,R_IY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x31},
   {R_IY,NOREQ,R_IX,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x31},
   {RS_IXY,NOREQ,RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0x2A},
   {R_IX,INDIRECT,R_IX,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x3F},
   {R_IY,INDIRECT,R_IY,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x3F},
   {R_IX,INDIRECT,R_IY,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x3E},
   {R_IY,INDIRECT,R_IX,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x3E},
   {RS_IXY,INDIRECT,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x36},
   {RS_IXY,INDIRECT,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,true,false,false,true,S_ANY,0x00,0x70},
   {R_MB,NOREQ,R_A,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x6D},
   {RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,RS_RR,NOREQ,TRANSFORM_NONE,TRANSFORM_P,false,false,false,false,S_ANY,0xED,0x43},
   {RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,R_SP,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x73},
   {R_R,NOREQ,R_A,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x4F},
   {RS_R,NOREQ,R_HL,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x46},
   {RS_AE,NOREQ,RS_IR,NOREQ,TRANSFORM_Y,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0x44},
   {RS_R,NOREQ,RS_IXY,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x46},
   {RS_RR,NOREQ,RS_IXY,INDIRECT,TRANSFORM_P,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x07},
   {RS_RR,NOREQ,RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,TRANSFORM_P,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x4B},
   {R_HL,INDIRECT,R_A,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x77},
   {R_SP,NOREQ,R_HL,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xF9},
   {R_SP,NOREQ,RS_IXY,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0xF9},
   {R_SP,NOREQ,RS_NONE,IMMEDIATE | IMM_MMN,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0x31},
   {R_SP,NOREQ,RS_NONE,INDIRECT_IMMEDIATE | IMM_MMN,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x7B},
};
operandlist_t operands_ldd[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xA8},
};
operandlist_t operands_lddr[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xB8},
};
operandlist_t operands_ldi[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xA0},
};
operandlist_t operands_ldir[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xB0},
};
operandlist_t operands_lea[] = {
// optimized set
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_RR,NOREQ,R_IX,NOREQ,TRANSFORM_P,TRANSFORM_NONE,false,true,false,false,S_ANY,0xED,0x02},
// end optimized set
   {R_IX,NOREQ,R_IX,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,false,S_ANY,0xED,0x32},
   {R_IY,NOREQ,R_IX,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,false,S_ANY,0xED,0x55},
   {R_IX,NOREQ,R_IY,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,false,S_ANY,0xED,0x54},
   {R_IY,NOREQ,R_IY,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,false,S_ANY,0xED,0x33},
   {RS_RR,NOREQ,R_IY,NOREQ,TRANSFORM_P,TRANSFORM_NONE,false,true,false,false,S_ANY,0xED,0x03},
};
operandlist_t operands_mlt[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_RR,NOREQ,RS_NONE,NOREQ,TRANSFORM_P,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x4C},
   {R_SP,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x7C},
};
operandlist_t operands_neg[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x44},
};
operandlist_t operands_nop[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x00},
};
operandlist_t operands_or[] = {
// optimized set
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xB0},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xF6},
// end optimized set
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xB6},
   {R_A,NOREQ,RS_IR,NOREQ,TRANSFORM_NONE,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0xB4},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0xB6},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xF6},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0xB0},
// same set, without A register
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xB6},
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR0,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0xB4},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0xB6},
};
operandlist_t operands_otd2r[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xBC},
};
operandlist_t operands_otdm[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x8B},
};
operandlist_t operands_otdmr[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x9B},
};
operandlist_t operands_otdr[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xBB},
};
operandlist_t operands_otdrx[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xCB},
};
operandlist_t operands_oti2r[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xB4},
};
operandlist_t operands_otim[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x83},
};
operandlist_t operands_otimr[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x93},
};
operandlist_t operands_otir[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xB3},
};
operandlist_t operands_otirx[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xC3},
};
operandlist_t operands_out[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_BC,INDIRECT,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Y,false,false,false,false,S_NONE,0xED,0x41},
   {R_C,INDIRECT,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Y,false,false,false,false,S_NONE,0xED,0x41},
   {RS_NONE,INDIRECT_IMMEDIATE | IMM_N,R_A,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xD3},
};
operandlist_t operands_out0[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,INDIRECT_IMMEDIATE | IMM_N,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Y,false,false,false,false,S_NONE,0xED,0x01},
};
operandlist_t operands_outd[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xAB},
};
operandlist_t operands_outd2[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xAC},
};
operandlist_t operands_outi[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xA3},
};
operandlist_t operands_outi2[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0xA4},
};
operandlist_t operands_pea[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_IX,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,false,S_ANY,0xED,0x65},
   {R_IY,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,false,S_ANY,0xED,0x66},
};
operandlist_t operands_pop[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_RR,NOREQ,RS_NONE,NOREQ,TRANSFORM_P,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xC1},
   {R_AF,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xF1},
   {RS_IXY,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0xE1},
};
operandlist_t operands_push[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_RR,NOREQ,RS_NONE,NOREQ,TRANSFORM_P,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xC5},
   {R_AF,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xF5},
   {RS_IXY,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,true,S_ANY,0x00,0xE5},
};
operandlist_t operands_res[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,IMMEDIATE | IMM_BIT,R_HL,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x86},
   {RS_NONE,IMMEDIATE | IMM_BIT,RS_IXY,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,true,false,true,S_ANY,0xCB,0x86},
   {RS_NONE,IMMEDIATE | IMM_BIT,RS_R,NOREQ,TRANSFORM_BIT,TRANSFORM_Z,false,false,false,false,S_NONE,0xCB,0x80},
};
operandlist_t operands_ret[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_LILLIS,0x00,0xC9},
   {RS_NONE,CC,RS_NONE,NOREQ,TRANSFORM_CC,TRANSFORM_NONE,false,false,true,false,S_LILLIS,0x00,0xC0},
};
operandlist_t operands_reti[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_LILLIS,0xED,0x4D},
};
operandlist_t operands_retn[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_LILLIS,0xED,0x45},
};
operandlist_t operands_rl[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x16},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0xCB,0x16},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0xCB,0x10},
};
operandlist_t operands_rla[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x17},
};
operandlist_t operands_rlc[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x06},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0xCB,0x06},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0xCB,0x00},
};
operandlist_t operands_rlca[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x07},
};
operandlist_t operands_rld[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x6F},
};
operandlist_t operands_rr[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x1E},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0xCB,0x1E},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0xCB,0x18},
};
operandlist_t operands_rra[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x1F},
};
operandlist_t operands_rrc[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x0E},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0xCB,0x0E},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0xCB,0x08},
};
operandlist_t operands_rrca[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x0F},
};
operandlist_t operands_rrd[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x67},
};
operandlist_t operands_rsmix[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x7E},
};
operandlist_t operands_rst[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_N,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xC7},
};
operandlist_t operands_sbc[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x9E},
   {R_A,NOREQ,RS_IR,NOREQ,TRANSFORM_NONE,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0x9C},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x9E},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xDE},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0x98},
// same set, without A register
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x9E},
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR0,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0x9C},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x9E},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xDE},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x98},

   {R_HL,NOREQ,RS_RR,NOREQ,TRANSFORM_NONE,TRANSFORM_P,false,false,false,false,S_ANY,0xED,0x42},
   {R_HL,NOREQ,R_SP,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x72},
};
operandlist_t operands_scf[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x37},
};
operandlist_t operands_set[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,IMMEDIATE | IMM_BIT,R_HL,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0xC6},
   {RS_NONE,IMMEDIATE | IMM_BIT,RS_IXY,INDIRECT,TRANSFORM_Y,TRANSFORM_NONE,false,true,false,true,S_ANY,0xCB,0xC6},
   {RS_NONE,IMMEDIATE | IMM_BIT,RS_R,NOREQ,TRANSFORM_BIT,TRANSFORM_Z,false,false,false,false,S_NONE,0xCB,0xC0},
};
operandlist_t operands_sla[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x26},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0xCB,0x26},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0xCB,0x20},
};
operandlist_t operands_slp[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x76},
};
operandlist_t operands_sra[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x2E},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0xCB,0x2E},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0xCB,0x28},
};
operandlist_t operands_srl[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xCB,0x3E},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0xCB,0x3E},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0xCB,0x38},
};
operandlist_t operands_stmix[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,NOREQ,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x7D},
};
operandlist_t operands_sub[] = {
// optimized set
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x96},
// end optimized set
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0x96},
   {R_A,NOREQ,RS_IR,NOREQ,TRANSFORM_NONE,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0x94},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0x96},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xD6},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0x90},
// same set, without A register
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR0,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0x94},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0x96},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xD6},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0x90},
};
operandlist_t operands_tst[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x34},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x64},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Y,false,false,false,false,S_NONE,0xED,0x04},
// same set, without A register
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0xED,0x34},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x64},
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Y,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x04},
};
operandlist_t operands_tstio[] = {
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0xED,0x74},
};
operandlist_t operands_xor[] = {
// optimized set
//     REGA       ADDRESSMODEA    REGB       ADDRESSMODEB        LENA    LENB       TRANSFORMA     TRANSFORMB DISPA DISPB    CC  DDFD      ADL PREF CODE
   {RS_R,NOREQ,RS_NONE,NOREQ,TRANSFORM_Z,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xA8},
   {RS_NONE,IMMEDIATE | IMM_N,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xEE},
   {R_HL,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xAE},
// end optimized set
   {R_A,NOREQ,R_HL,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_ANY,0x00,0xAE},
   {R_A,NOREQ,RS_IR,NOREQ,TRANSFORM_NONE,TRANSFORM_IR0,false,false,false,true,S_NONE,0x00,0xAC},
   {R_A,NOREQ,RS_IXY,INDIRECT,TRANSFORM_NONE,TRANSFORM_NONE,false,true,false,true,S_ANY,0x00,0xAE},
   {R_A,NOREQ,RS_NONE,IMMEDIATE | IMM_N,TRANSFORM_NONE,TRANSFORM_NONE,false,false,false,false,S_NONE,0x00,0xEE},
   {R_A,NOREQ,RS_R,NOREQ,TRANSFORM_NONE,TRANSFORM_Z,false,false,false,false,S_NONE,0x00,0xA8},
// same set, without A register
   {RS_IR,NOREQ,RS_NONE,NOREQ,TRANSFORM_IR0,TRANSFORM_NONE,false,false,false,true,S_NONE,0x00,0xAC},
   {RS_IXY,INDIRECT,RS_NONE,NOREQ,TRANSFORM_NONE,TRANSFORM_NONE,true,false,false,true,S_ANY,0x00,0xAE},
};

instruction_t instructions[] = {
    {"adc",      EZ80, 0, sizeof(operands_adc)/sizeof(operandlist_t), operands_adc , 0},
    {"add",      EZ80, 0, sizeof(operands_add)/sizeof(operandlist_t), operands_add , 0},
    {"align",    ASSEMBLER, ASM_ALIGN, 0, NULL, ASM_ARG_SINGLE},
    {"and",      EZ80, 0, sizeof(operands_and)/sizeof(operandlist_t), operands_and , 0},
    {"ascii",    ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"asciz",    ASSEMBLER, ASM_ASCIZ, 0, NULL, ASM_ARG_LIST},
    {"assume",   ASSEMBLER, ASM_ADL, 0, NULL, ASM_ARG_KEYVAL},
    {"bit",      EZ80, 0, sizeof(operands_bit)/sizeof(operandlist_t), operands_bit , 0},
    {"blkb",     ASSEMBLER, ASM_BLKB, 0, NULL, ASM_ARG_LIST},
    {"blkl",     ASSEMBLER, ASM_BLKL, 0, NULL, ASM_ARG_LIST},
    {"blkp",     ASSEMBLER, ASM_BLKP, 0, NULL, ASM_ARG_LIST},
    {"blkw",     ASSEMBLER, ASM_BLKW, 0, NULL, ASM_ARG_LIST},
    {"byte",     ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"call",     EZ80, 0, sizeof(operands_call)/sizeof(operandlist_t), operands_call , 0},
    {"ccf",      EZ80, 0, sizeof(operands_ccf)/sizeof(operandlist_t), operands_ccf , 0},
    {"cp",       EZ80, 0, sizeof(operands_cp)/sizeof(operandlist_t), operands_cp , 0},
    {"cpd",      EZ80, 0, sizeof(operands_cpd)/sizeof(operandlist_t), operands_cpd , 0},
    {"cpdr",     EZ80, 0, sizeof(operands_cpdr)/sizeof(operandlist_t), operands_cpdr , 0},
    {"cpi",      EZ80, 0, sizeof(operands_cpi)/sizeof(operandlist_t), operands_cpi , 0},
    {"cpir",     EZ80, 0, sizeof(operands_cpir)/sizeof(operandlist_t), operands_cpir , 0},
    {"cpl",      EZ80, 0, sizeof(operands_cpl)/sizeof(operandlist_t), operands_cpl , 0},
    {"daa",      EZ80, 0, sizeof(operands_daa)/sizeof(operandlist_t), operands_daa , 0},
    {"db",       ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"dec",      EZ80, 0, sizeof(operands_dec)/sizeof(operandlist_t), operands_dec , 0},
    {"defb",     ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"defs",     ASSEMBLER, ASM_DS, 0, NULL, ASM_ARG_LIST},
    {"defw",     ASSEMBLER, ASM_DW, 0, NULL, ASM_ARG_LIST},
    {"di",       EZ80, 0, sizeof(operands_di)/sizeof(operandlist_t), operands_di , 0},
    {"djnz",     EZ80, 0, sizeof(operands_djnz)/sizeof(operandlist_t), operands_djnz , 0},
    {"dl",       ASSEMBLER, ASM_DW24, 0, NULL, ASM_ARG_LIST},
    {"ds",       ASSEMBLER, ASM_DS, 0, NULL, ASM_ARG_LIST},
    {"dw",       ASSEMBLER, ASM_DW, 0, NULL, ASM_ARG_LIST},
    {"dw24",     ASSEMBLER, ASM_DW24, 0, NULL, ASM_ARG_LIST},
    {"dw32",     ASSEMBLER, ASM_DW32, 0, NULL, ASM_ARG_LIST},
    {"ei",       EZ80, 0, sizeof(operands_ei)/sizeof(operandlist_t), operands_ei , 0},
    {"else",     ASSEMBLER, ASM_ELSE, 0, NULL, ASM_ARG_NONE},
    {"endif",    ASSEMBLER, ASM_ENDIF, 0, NULL, ASM_ARG_NONE},
    {"endmacro", ASSEMBLER, ASM_MACRO_END, 0, NULL, ASM_ARG_SINGLE},
    {"equ",      ASSEMBLER, ASM_EQU, 0, NULL, ASM_ARG_SINGLE},
    {"ex",       EZ80, 0, sizeof(operands_ex)/sizeof(operandlist_t), operands_ex , 0},
    {"exx",      EZ80, 0, sizeof(operands_exx)/sizeof(operandlist_t), operands_exx , 0},
    {"fillbyte", ASSEMBLER, ASM_FILLBYTE, 0, NULL, ASM_ARG_SINGLE},
    {"halt",     EZ80, 0, sizeof(operands_halt)/sizeof(operandlist_t), operands_halt , 0},
    {"if",       ASSEMBLER, ASM_IF, 0, NULL, ASM_ARG_SINGLE},
    {"im",       EZ80, 0, sizeof(operands_im)/sizeof(operandlist_t), operands_im , 0},
    {"in",       EZ80, 0, sizeof(operands_in)/sizeof(operandlist_t), operands_in , 0},
    {"in0",      EZ80, 0, sizeof(operands_in0)/sizeof(operandlist_t), operands_in0 , 0},
    {"inc",      EZ80, 0, sizeof(operands_inc)/sizeof(operandlist_t), operands_inc , 0},
    {"incbin",   ASSEMBLER, ASM_INCBIN, 0, NULL, ASM_ARG_SINGLE},
    {"include",  ASSEMBLER, ASM_INCLUDE, 0, NULL, ASM_ARG_SINGLE},
    {"ind",      EZ80, 0, sizeof(operands_ind)/sizeof(operandlist_t), operands_ind , 0},
    {"ind2",     EZ80, 0, sizeof(operands_ind2)/sizeof(operandlist_t), operands_ind2 , 0},
    {"ind2r",    EZ80, 0, sizeof(operands_ind2r)/sizeof(operandlist_t), operands_ind2r , 0},
    {"indm",     EZ80, 0, sizeof(operands_indm)/sizeof(operandlist_t), operands_indm , 0},
    {"indmr",    EZ80, 0, sizeof(operands_indmr)/sizeof(operandlist_t), operands_indmr , 0},
    {"indr",     EZ80, 0, sizeof(operands_indr)/sizeof(operandlist_t), operands_indr , 0},
    {"indrx",    EZ80, 0, sizeof(operands_indrx)/sizeof(operandlist_t), operands_indrx , 0},
    {"ini",      EZ80, 0, sizeof(operands_ini)/sizeof(operandlist_t), operands_ini , 0},
    {"ini2",     EZ80, 0, sizeof(operands_ini2)/sizeof(operandlist_t), operands_ini2 , 0},
    {"ini2r",    EZ80, 0, sizeof(operands_ini2r)/sizeof(operandlist_t), operands_ini2r , 0},
    {"inim",     EZ80, 0, sizeof(operands_inim)/sizeof(operandlist_t), operands_inim , 0},
    {"inimr",    EZ80, 0, sizeof(operands_inimr)/sizeof(operandlist_t), operands_inimr , 0},
    {"inir",     EZ80, 0, sizeof(operands_inir)/sizeof(operandlist_t), operands_inir , 0},
    {"inirx",    EZ80, 0, sizeof(operands_inirx)/sizeof(operandlist_t), operands_inirx , 0},
    {"jp",       EZ80, 0, sizeof(operands_jp)/sizeof(operandlist_t), operands_jp , 0},
    {"jr",       EZ80, 0, sizeof(operands_jr)/sizeof(operandlist_t), operands_jr , 0},
    {"ld",       EZ80, 0, sizeof(operands_ld)/sizeof(operandlist_t), operands_ld , 0},
    {"ldd",      EZ80, 0, sizeof(operands_ldd)/sizeof(operandlist_t), operands_ldd , 0},
    {"lddr",     EZ80, 0, sizeof(operands_lddr)/sizeof(operandlist_t), operands_lddr , 0},
    {"ldi",      EZ80, 0, sizeof(operands_ldi)/sizeof(operandlist_t), operands_ldi , 0},
    {"ldir",     EZ80, 0, sizeof(operands_ldir)/sizeof(operandlist_t), operands_ldir , 0},
    {"lea",      EZ80, 0, sizeof(operands_lea)/sizeof(operandlist_t), operands_lea , 0},
    {"macro",    ASSEMBLER, ASM_MACRO_START, 0, NULL, ASM_ARG_LIST},
    {"mlt",      EZ80, 0, sizeof(operands_mlt)/sizeof(operandlist_t), operands_mlt , 0},
    {"neg",      EZ80, 0, sizeof(operands_neg)/sizeof(operandlist_t), operands_neg , 0},
    {"nop",      EZ80, 0, sizeof(operands_nop)/sizeof(operandlist_t), operands_nop , 0},
    {"or",       EZ80, 0, sizeof(operands_or)/sizeof(operandlist_t), operands_or , 0},
    {"org",      ASSEMBLER, ASM_ORG, 0, NULL, ASM_ARG_SINGLE},
    {"otd2r",    EZ80, 0, sizeof(operands_otd2r)/sizeof(operandlist_t), operands_otd2r , 0},
    {"otdm",     EZ80, 0, sizeof(operands_otdm)/sizeof(operandlist_t), operands_otdm , 0},
    {"otdmr",    EZ80, 0, sizeof(operands_otdmr)/sizeof(operandlist_t), operands_otdmr , 0},
    {"otdr",     EZ80, 0, sizeof(operands_otdr)/sizeof(operandlist_t), operands_otdr , 0},
    {"otdrx",    EZ80, 0, sizeof(operands_otdrx)/sizeof(operandlist_t), operands_otdrx , 0},
    {"oti2r",    EZ80, 0, sizeof(operands_oti2r)/sizeof(operandlist_t), operands_oti2r , 0},
    {"otim",     EZ80, 0, sizeof(operands_otim)/sizeof(operandlist_t), operands_otim , 0},
    {"otimr",    EZ80, 0, sizeof(operands_otimr)/sizeof(operandlist_t), operands_otimr , 0},
    {"otir",     EZ80, 0, sizeof(operands_otir)/sizeof(operandlist_t), operands_otir , 0},
    {"otirx",    EZ80, 0, sizeof(operands_otirx)/sizeof(operandlist_t), operands_otirx , 0},
    {"out",      EZ80, 0, sizeof(operands_out)/sizeof(operandlist_t), operands_out , 0},
    {"out0",     EZ80, 0, sizeof(operands_out0)/sizeof(operandlist_t), operands_out0 , 0},
    {"outd",     EZ80, 0, sizeof(operands_outd)/sizeof(operandlist_t), operands_outd , 0},
    {"outd2",    EZ80, 0, sizeof(operands_outd2)/sizeof(operandlist_t), operands_outd2 , 0},
    {"outi",     EZ80, 0, sizeof(operands_outi)/sizeof(operandlist_t), operands_outi , 0},
    {"outi2",    EZ80, 0, sizeof(operands_outi2)/sizeof(operandlist_t), operands_outi2 , 0},
    {"pea",      EZ80, 0, sizeof(operands_pea)/sizeof(operandlist_t), operands_pea , 0},
    {"pop",      EZ80, 0, sizeof(operands_pop)/sizeof(operandlist_t), operands_pop , 0},
    {"push",     EZ80, 0, sizeof(operands_push)/sizeof(operandlist_t), operands_push , 0},
    {"res",      EZ80, 0, sizeof(operands_res)/sizeof(operandlist_t), operands_res , 0},
    {"ret",      EZ80, 0, sizeof(operands_ret)/sizeof(operandlist_t), operands_ret , 0},
    {"reti",     EZ80, 0, sizeof(operands_reti)/sizeof(operandlist_t), operands_reti , 0},
    {"retn",     EZ80, 0, sizeof(operands_retn)/sizeof(operandlist_t), operands_retn , 0},
    {"rl",       EZ80, 0, sizeof(operands_rl)/sizeof(operandlist_t), operands_rl , 0},
    {"rla",      EZ80, 0, sizeof(operands_rla)/sizeof(operandlist_t), operands_rla , 0},
    {"rlc",      EZ80, 0, sizeof(operands_rlc)/sizeof(operandlist_t), operands_rlc , 0},
    {"rlca",     EZ80, 0, sizeof(operands_rlca)/sizeof(operandlist_t), operands_rlca , 0},
    {"rld",      EZ80, 0, sizeof(operands_rld)/sizeof(operandlist_t), operands_rld , 0},
    {"rr",       EZ80, 0, sizeof(operands_rr)/sizeof(operandlist_t), operands_rr , 0},
    {"rra",      EZ80, 0, sizeof(operands_rra)/sizeof(operandlist_t), operands_rra , 0},
    {"rrc",      EZ80, 0, sizeof(operands_rrc)/sizeof(operandlist_t), operands_rrc , 0},
    {"rrca",     EZ80, 0, sizeof(operands_rrca)/sizeof(operandlist_t), operands_rrca , 0},
    {"rrd",      EZ80, 0, sizeof(operands_rrd)/sizeof(operandlist_t), operands_rrd , 0},
    {"rsmix",    EZ80, 0, sizeof(operands_rsmix)/sizeof(operandlist_t), operands_rsmix , 0},
    {"rst",      EZ80, 0, sizeof(operands_rst)/sizeof(operandlist_t), operands_rst , 0},
    {"sbc",      EZ80, 0, sizeof(operands_sbc)/sizeof(operandlist_t), operands_sbc , 0},
    {"scf",      EZ80, 0, sizeof(operands_scf)/sizeof(operandlist_t), operands_scf , 0},
    {"set",      EZ80, 0, sizeof(operands_set)/sizeof(operandlist_t), operands_set , 0},
    {"sla",      EZ80, 0, sizeof(operands_sla)/sizeof(operandlist_t), operands_sla , 0},
    {"slp",      EZ80, 0, sizeof(operands_slp)/sizeof(operandlist_t), operands_slp , 0},
    {"sra",      EZ80, 0, sizeof(operands_sra)/sizeof(operandlist_t), operands_sra , 0},
    {"srl",      EZ80, 0, sizeof(operands_srl)/sizeof(operandlist_t), operands_srl , 0},
    {"stmix",    EZ80, 0, sizeof(operands_stmix)/sizeof(operandlist_t), operands_stmix , 0},
    {"sub",      EZ80, 0, sizeof(operands_sub)/sizeof(operandlist_t), operands_sub , 0},
    {"tst",      EZ80, 0, sizeof(operands_tst)/sizeof(operandlist_t), operands_tst , 0},
    {"tstio",    EZ80, 0, sizeof(operands_tstio)/sizeof(operandlist_t), operands_tstio , 0},
    {"xor",      EZ80, 0, sizeof(operands_xor)/sizeof(operandlist_t), operands_xor , 0}
};

instruction_t * instruction_hashtable_lookup(char *name) {
    int index,i,try;

    index = lowercaseHash(name) % INSTRUCTION_HASHTABLESIZE;
    for(i = 0; i < INSTRUCTION_HASHTABLESIZE; i++){
        try = (index + i) % INSTRUCTION_HASHTABLESIZE;
        if(instruction_hashtable[try] == NULL){
            return NULL;
        }
        if(instruction_hashtable[try] != NULL &&
            fast_strcasecmp(instruction_hashtable[try]->name,name) == 0) {
            return instruction_hashtable[try];
        }
    }
    return NULL;
}

void init_instruction_hashtable(void) {
    uint16_t n, h;

    memset(instruction_hashtable, 0, sizeof(instruction_hashtable));

    for(n = 0; n < (sizeof(instructions) / sizeof(instruction_t)); n++) {
        h = hash(instructions[n].name) % INSTRUCTION_HASHTABLESIZE;
        while(instruction_hashtable[h]) {
            h = h + 1;
        }
        instruction_hashtable[h] = &instructions[n];
    }
}
