#include "instruction.h"
#include "moscalls.h"
#include "globals.h"
#include "label.h"
#include "io.h"

// instruction hash table
instruction_t *instruction_hashtable[INSTRUCTION_HASHTABLESIZE];

bool none_match(operand_t *op) {
    return ((op->reg == R_NONE) && (op->immediate_provided == false) & !(op->cc));
}
bool cc_match(operand_t *op) {
    return op->cc;
}
bool ir_match(operand_t *op) {
    return ((op->reg >= R_IXH) && (op->reg <= R_IYL) && !(op->indirect));
}
bool ixy_match(operand_t *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && !(op->indirect)  && !(op->displacement_provided));
}
bool ixyd_match(operand_t *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && !(op->indirect));
}
bool indirect_ixyd_match(operand_t *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && op->indirect);
}
bool mmn_match(operand_t *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool indirect_mmn_match(operand_t *op) {
    return ((op->indirect) && (op->immediate_provided));
}
bool n_match(operand_t *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool a_match(operand_t *op) {
    return(op->reg == R_A);
}
bool hl_match(operand_t *op) {
    return((op->reg == R_HL) && !(op->indirect));
}
bool indirect_hl_match(operand_t *op) {
    return((op->reg == R_HL) && (op->indirect));
}
bool rr_match(operand_t *op) {
    return((op->reg >= R_BC) && (op->reg <= R_HL) && !(op->indirect));
}
bool indirect_rr_match(operand_t *op) {
    return((op->reg >= R_BC) && (op->reg <= R_HL) && (op->indirect));
}
bool rxy_match(operand_t *op) {
    return(!(op->indirect) && ((op->reg == R_BC) || (op->reg == R_DE) || (op->reg == R_IX) || (op->reg == R_IY)));
}
bool sp_match(operand_t *op) {
    return(!(op->indirect) && (op->reg == R_SP));
}
bool indirect_sp_match(operand_t *op) {
    return((op->indirect) && (op->reg == R_SP));
}
bool r_match(operand_t *op) {
    return((op->reg >= R_A) && (op->reg <= R_L));
}
bool reg_r_match(operand_t *op) {
    return(op->reg == R_R);
}
bool mb_match(operand_t *op) {
    return(op->reg == R_MB);
}
bool i_match(operand_t *op) {
    return(op->reg == R_I);
}
bool b_match(operand_t *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool af_match(operand_t *op) {
    return(op->reg == R_AF);
}
bool de_match(operand_t *op) {
    return(op->reg == R_DE);
}
bool nselect_match(operand_t *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool indirect_n_match(operand_t *op) {
    return ((op->indirect) && (op->immediate_provided));
}
bool indirect_bc_match(operand_t *op) {
    return((op->indirect) && (op->reg == R_BC));
}
bool indirect_c_match(operand_t *op) {
    return((op->indirect) && (op->reg == R_C));
}
bool indirect_ixy_match(operand_t *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && (op->indirect)  && !(op->displacement_provided));
}
bool cca_match(operand_t *op) {
    return ((op->cc) && 
            ((op->cc_index == CC_INDEX_NZ) ||
             (op->cc_index == CC_INDEX_Z) ||
             (op->cc_index == CC_INDEX_NC) ||
             (op->cc_index == CC_INDEX_C)));
}
bool indirect_de_match(operand_t *op) {
    return((op->indirect) && (op->reg == R_DE));
}
bool ix_match(operand_t *op) {
    return(!(op->indirect) && (op->reg == R_IX));
}
bool iy_match(operand_t *op) {
    return(!(op->indirect) && (op->reg == R_IY));
}
bool ixd_match(operand_t *op) {
    return(!(op->indirect) && (op->reg == R_IX));
}
bool iyd_match(operand_t *op) {
    return(!(op->indirect) && (op->reg == R_IY));
}
bool indirect_ixd_match(operand_t *op) {
    return((op->indirect) && (op->reg == R_IX));
}
bool indirect_iyd_match(operand_t *op) {
    return((op->indirect) && (op->reg == R_IY));
}
bool raeonly_match(operand_t *op) {
    return((op->reg >= R_A) && (op->reg <= R_E));
}

// get the number of bytes to emit from an immediate
uint8_t get_immediate_size(operand_t *op, uint8_t suffix) {
    uint8_t num;
    switch(suffix) {
        case S_SIS:
        case S_LIS:
            num = 2;
            break;
        case S_SIL:
        case S_LIL:
            num = 3;
            break;
        case 0: // Use current ADL mode to determine 16/24 bit
            if(adlmode) num = 3;
            else num = 2;
            break;
        default:
            error(message[ERROR_INVALIDMNEMONIC]);
            return 0;
    }
    if(num == 2) op->immediate &= 0xFFFF;
    return num;
}

uint8_t get_ddfd_prefix(cpuregister reg) {
    switch(reg) {
        case R_IX:
        case R_IXH:
        case R_IXL:
            return 0xDD;
        case R_IY:
        case R_IYH:
        case R_IYL:
            return 0xFD;
        default:
            break;
    }
    return 0;    
}

void prefix_ddfd_suffix(operandlist_t *op) {
    uint8_t prefix1, prefix2;

    if(!op->ddfdpermitted) return;

    prefix1 = get_ddfd_prefix(operand1.reg);
    prefix2 = get_ddfd_prefix(operand2.reg);

    // prefix in either of these two cases
    if(prefix1) {
        if(prefix2) {
            // both prefixes set
            if(operand1.indirect) output.prefix1 = prefix1;
            else output.prefix1 = prefix2;
        }
        else output.prefix1 = prefix1;
    }
    else if(prefix2) output.prefix1 = prefix2;
}

void transform_instruction(operand_t *op, permittype_t type) {
    uint8_t y;
    int24_t rel;

    switch(type) {
        case TRANSFORM_IR0:
            if((op->reg == R_IXL) || (op->reg == R_IYL)) output.opcode |= 0x01;
            break;
        case TRANSFORM_IR3:
            if((op->reg == R_IXL) || (op->reg == R_IYL)) output.opcode |= 0x08;
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
    bool op1_displacement_required = false;
    bool op2_displacement_required = false;

    // Transform necessary prefix/opcode in output, according to given list and operands
    output.suffix = getADLsuffix();
    output.prefix1 = 0;
    output.prefix2 = list->prefix;
    output.opcode = list->opcode;

    //if(pass == 1) definelabel(address);
    definelabel(address);

    // Output displacement if needed, even when none is given (handles implicit cases)
    if(list->operandA > OPTYPE_R_AEONLY) op1_displacement_required = true;
    if(list->operandB > OPTYPE_R_AEONLY) op2_displacement_required = true;

    // issue any errors here
    if((list->transformA != TRANSFORM_REL) && (list->transformB != TRANSFORM_REL)) { // TRANSFORM_REL will mask to 0xFF
        if(((list->operandA == OPTYPE_N) || (list->operandA == OPTYPE_INDIRECT_N)) && ((operand1.immediate > 0xFF) || (operand1.immediate < -128))) error(message[ERROR_8BITRANGE]);
        if(((list->operandB == OPTYPE_N) || (list->operandB == OPTYPE_INDIRECT_N)) && ((operand2.immediate > 0xFF) || (operand2.immediate < -128))) error(message[ERROR_8BITRANGE]);
    }
    if((output.suffix) && ((list->adl & output.suffix) == 0)) error(message[ERROR_ILLEGAL_SUFFIXMODE]);
    if((op2_displacement_required) && ((operand2.displacement < -128) || (operand2.displacement > 127))) error(message[ERROR_DISPLACEMENT_RANGE]);

    // Specific checks
    if((list->operandA == OPTYPE_BIT) && (operand1.immediate > 7)) error(message[ERROR_INVALIDBITNUMBER]);
    if((list->operandA == OPTYPE_NSELECT) && (operand1.immediate > 2)) error(message[ERROR_ILLEGALINTERRUPTMODE]);
    if((list->transformA == TRANSFORM_N) && (operand1.immediate & 0x47)) error(message[ERROR_ILLEGALRESTARTADDRESS]);

    // prepare extra DD/FD suffix if needed
    prefix_ddfd_suffix(list);
    // Transform the opcode and potential immediate values, according to the current ruleset
    transform_instruction(&operand1, (permittype_t)list->transformA);
    transform_instruction(&operand2, (permittype_t)list->transformB);
    // determine position of dd
    ddbeforeopcode = (((output.prefix1 == 0xDD) || (output.prefix1 == 0xFD)) && (output.prefix2 == 0xCB) &&
                ((op1_displacement_required) || (op2_displacement_required)));
    
    // output adl suffix and any prefixes
    if(output.suffix > 0) emit_adlsuffix_code(output.suffix);
    if(output.prefix1) emit_8bit(output.prefix1);
    if(output.prefix2) emit_8bit(output.prefix2);

    // opcode in normal position
    if(!ddbeforeopcode) emit_8bit(output.opcode);
    
    // output displacement
    if(op1_displacement_required) emit_8bit(operand1.displacement & 0xFF);
    if(op2_displacement_required) emit_8bit(operand2.displacement & 0xFF);
    
    // output n
    if((operand1.immediate_provided) && ((list->operandA == OPTYPE_N) || (list->operandA == OPTYPE_INDIRECT_N))) emit_8bit(operand1.immediate & 0xFF);
    if((operand2.immediate_provided) && ((list->operandB == OPTYPE_N) || (list->operandB == OPTYPE_INDIRECT_N))) emit_8bit(operand2.immediate & 0xFF);

    // opcode in DDCBdd/DFCBdd position
    if(ddbeforeopcode) emit_8bit(output.opcode);

    //output remaining immediate bytes
    if((list->operandA == OPTYPE_MMN) || (list->operandA == OPTYPE_INDIRECT_MMN)) emit_immediate(&operand1, output.suffix);
    if((list->operandB == OPTYPE_MMN) || (list->operandB == OPTYPE_INDIRECT_MMN)) emit_immediate(&operand2, output.suffix);
}

// table with fast access to functions that perform matching to an specific permittype
permittype_match_t permittype_matchlist[] = {
    {OPTYPE_NONE,           none_match},
    {OPTYPE_CC,             cc_match},
    {OPTYPE_IR,             ir_match},
    {OPTYPE_IXY,            ixy_match},
    {OPTYPE_MMN,            mmn_match},
    {OPTYPE_INDIRECT_MMN,   indirect_mmn_match},
    {OPTYPE_N,              n_match},
    {OPTYPE_A,              a_match},
    {OPTYPE_HL,             hl_match},
    {OPTYPE_INDIRECT_HL,    indirect_hl_match},
    {OPTYPE_RR,             rr_match},
    {OPTYPE_INDIRECT_RR,    indirect_rr_match},
    {OPTYPE_RXY,            rxy_match},
    {OPTYPE_SP,             sp_match},
    {OPTYPE_INDIRECT_SP,    indirect_sp_match},
    {OPTYPE_R,              r_match},
    {OPTYPE_REG_R,          reg_r_match},
    {OPTYPE_MB,             mb_match},
    {OPTYPE_I,              i_match},
    {OPTYPE_BIT,            b_match},
    {OPTYPE_AF,             af_match},
    {OPTYPE_DE,             de_match},
    {OPTYPE_NSELECT,        nselect_match},
    {OPTYPE_INDIRECT_N,     indirect_n_match},
    {OPTYPE_INDIRECT_BC,    indirect_bc_match},
    {OPTYPE_INDIRECT_C,     indirect_c_match},
    {OPTYPE_INDIRECT_IXY,   indirect_ixy_match},
    {OPTYPE_CCA,            cca_match},
    {OPTYPE_INDIRECT_DE,    indirect_de_match},
    {OPTYPE_IX,             ix_match},
    {OPTYPE_IY,             iy_match},
    {OPTYPE_R_AEONLY,       raeonly_match},
    {OPTYPE_IXYd,           ixyd_match},
    {OPTYPE_INDIRECT_IXYd,  indirect_ixyd_match},
    {OPTYPE_IXd,            ixd_match},
    {OPTYPE_IYd,            iyd_match},
    {OPTYPE_INDIRECT_IXd,   indirect_ixd_match},
    {OPTYPE_INDIRECT_IYd,   indirect_iyd_match}
};

operandlist_t operands_adc[] = {
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_ANY},
    {R_A, NOREQ, RS_IR, NOREQ, OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x8C, S_NONE},
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_ANY},
    {R_A, NOREQ, RS_NONE, STATE_IMMEDIATE, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xCE, S_NONE},
    {R_A, NOREQ, R_R, NOREQ, OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x88, S_NONE},
    // same set, without A register
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_ANY},
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0x8C, S_NONE},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xCE, S_NONE},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0x88, S_NONE},

    {R_HL, NOREQ, RS_RR, NOREQ, OPTYPE_HL, OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x4A, S_ANY},
    {R_HL, NOREQ, R_SP, NOREQ, OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7A, S_ANY},
};
operandlist_t operands_add[] = {
// optimized set
    {R_HL, NOREQ, RS_RR, NOREQ, OPTYPE_HL,OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x09, S_ANY},
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x80, S_NONE},
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_ANY},
// end optimized set
    {R_A, NOREQ, RS_IR, NOREQ, OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x84, S_NONE},
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_ANY},
    {R_A, NOREQ, RS_NONE, NOREQ, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC6, S_NONE},
    // same set, without A register
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_ANY},
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0x84, S_NONE},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC6, S_NONE},

    {R_HL, NOREQ, R_SP, NOREQ, OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x39, S_ANY},
    {RS_IXY, NOREQ, RS_RXY, NOREQ, OPTYPE_IXY, OPTYPE_RXY,             true, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x09, S_ANY},
    {RS_IXY, NOREQ, R_SP, NOREQ, OPTYPE_IXY, OPTYPE_SP,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x39, S_ANY},
};
operandlist_t operands_and[] = {
// optimized set
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xA0, S_NONE},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE6, S_NONE},
// end optimized set
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_ANY},
    {R_A, NOREQ, RS_IR, NOREQ, OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xA4, S_NONE},
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_ANY},
    {R_A, NOREQ, RS_NONE, NOREQ, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE6, S_NONE},
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xA0, S_NONE},
    // same set, without A register
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_ANY},
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xA4, S_NONE},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_ANY},
};
operandlist_t operands_bit[] = {
    {RS_NONE, STATE_IMMEDIATE, R_HL, STATE_INDIRECT, OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x46, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_IXY, STATE_INDIRECT, OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x46, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_R, NOREQ, OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_Y,      TRANSFORM_Z,    0xCB, 0x40, S_NONE},
};
operandlist_t operands_call[] = {
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_MMN, OPTYPE_NONE ,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xCD, S_ANY},
    {RS_NONE, STATE_CC, RS_NONE, STATE_IMMEDIATE, OPTYPE_CC, OPTYPE_MMN,             false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC4, S_ANY},
};
operandlist_t operands_ccf[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_NONE},
};
operandlist_t operands_cp[]= {
// optimized set
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N,  OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFE, S_NONE},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,            false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xB8, S_NONE},
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_ANY},
    {R_A, NOREQ, RS_NONE, NOREQ, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFE, S_NONE},
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xB8, S_NONE},
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_ANY},
// end optimized set
    {R_A, NOREQ, RS_IR, NOREQ, OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xBC, S_NONE},
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_ANY},
    // same set, without A register
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR,  OPTYPE_NONE,            true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xBC, S_NONE},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ ,OPTYPE_INDIRECT_IXYd, OPTYPE_NONE, true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_ANY},
};
operandlist_t operands_cpd[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA9, S_ANY},
};
operandlist_t operands_cpdr[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB9, S_ANY},
};
operandlist_t operands_cpi[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA1, S_ANY},
};
operandlist_t operands_cpir[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB1, S_ANY},
};
operandlist_t operands_cpl[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2F, S_NONE},
};
operandlist_t operands_daa[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x27, S_NONE},
};
operandlist_t operands_dec[]= {
// optimized set
    {RS_RR, NOREQ, RS_NONE, NOREQ, OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x0B, S_ANY}, 
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x05, S_NONE}, 
// end optimized set
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x35, S_ANY}, 
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x25, S_NONE}, 
    {RS_IXY, NOREQ, RS_NONE, NOREQ, OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2B, S_ANY}, 
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x35, S_ANY}, 
    {R_SP, NOREQ, RS_NONE, NOREQ, OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3B, S_ANY}, 
};
operandlist_t operands_di[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF3, S_NONE},
};
operandlist_t operands_djnz[]= {
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_REL,    TRANSFORM_NONE, 0x00, 0x10, S_NONE},
};
operandlist_t operands_ei[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFB, S_NONE},
};
operandlist_t operands_ex[]= {
// optimized set
    {R_DE, NOREQ, R_HL, NOREQ, OPTYPE_DE, OPTYPE_HL,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEB, S_NONE},
// end optimized set
    {R_AF, NOREQ, R_AF, NOREQ, OPTYPE_AF, OPTYPE_AF,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x08, S_NONE},
    {R_SP, STATE_INDIRECT, R_HL, NOREQ, OPTYPE_INDIRECT_SP, OPTYPE_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE3, S_ANY},
    {R_SP, STATE_INDIRECT, RS_IXY, NOREQ, OPTYPE_INDIRECT_SP, OPTYPE_IXY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE3, S_ANY},
};
operandlist_t operands_exx[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD9, S_NONE},
};
operandlist_t operands_halt[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x76, S_NONE},
};
operandlist_t operands_im[]= {
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_NSELECT, OPTYPE_NONE,       false, TRANSFORM_SELECT, TRANSFORM_NONE, 0xED, 0x46, S_NONE}, 
};
operandlist_t operands_in[]= {
    {R_A, NOREQ, RS_NONE, STATE_IMMEDIATE | STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_N,       false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xDB, S_NONE}, 
    {RS_R, NOREQ, R_BC, STATE_INDIRECT, OPTYPE_R, OPTYPE_INDIRECT_BC,      false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x40, S_NONE}, 
    {RS_R, NOREQ, R_C, STATE_INDIRECT, OPTYPE_R, OPTYPE_INDIRECT_C,       false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x40, S_NONE}, 
};
operandlist_t operands_in0[]= {
    {RS_R, NOREQ, RS_NONE, STATE_IMMEDIATE | STATE_INDIRECT, OPTYPE_R, OPTYPE_INDIRECT_N,       false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x00, S_NONE}, 
};
operandlist_t operands_inc[]= {
// optimized set
    {RS_RR, NOREQ, RS_NONE, NOREQ, OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x03, S_ANY}, 
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x04, S_NONE}, 
    {RS_IXY, NOREQ, RS_NONE, NOREQ, OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x23, S_ANY}, 
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x34, S_ANY}, 
// end optimized set
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x24, S_NONE}, 
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x34, S_ANY}, 
    {R_SP, NOREQ, RS_NONE, NOREQ, OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x33, S_ANY}, 
};
operandlist_t operands_ind[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAA, S_ANY}, 
};
operandlist_t operands_ind2[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8C, S_ANY}, 
};
operandlist_t operands_ind2r[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9C, S_ANY}, 
};
operandlist_t operands_indm[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8A, S_ANY}, 
};
operandlist_t operands_indmr[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9A, S_ANY}, 
};
operandlist_t operands_indr[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBA, S_ANY}, 
};
operandlist_t operands_indrx[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xCA, S_ANY}, 
};
operandlist_t operands_ini[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA2, S_ANY}, 
};
operandlist_t operands_ini2[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x84, S_ANY}, 
};
operandlist_t operands_ini2r[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x94, S_ANY}, 
};
operandlist_t operands_inim[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x82, S_ANY}, 
};
operandlist_t operands_inimr[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x92, S_ANY}, 
};
operandlist_t operands_inir[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB2, S_ANY}, 
};
operandlist_t operands_inirx[]= {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC2, S_ANY}, 
};
operandlist_t operands_jp[] = {
    {RS_NONE, STATE_CC, RS_NONE, STATE_IMMEDIATE, OPTYPE_CC, OPTYPE_MMN,             false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC2, S_SISLIL},
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE9, S_ANY},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXY, OPTYPE_NONE,   true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE9, S_SISLIL},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_MMN, OPTYPE_NONE ,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC3, S_SISLIL},
};
operandlist_t operands_jr[]= {
    {RS_NONE, STATE_CCA, RS_NONE, STATE_IMMEDIATE, OPTYPE_CCA, OPTYPE_N,              false, TRANSFORM_CC,     TRANSFORM_REL,  0x00, 0x20, S_NONE},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_REL,    TRANSFORM_NONE, 0x00, 0x18, S_NONE},
};
operandlist_t operands_ld[] = {
// start optimized set
    {0,0,0,0,OPTYPE_RR, OPTYPE_MMN,              true, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x01, S_ANY},
    {0,0,0,0,OPTYPE_R, OPTYPE_N,                false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x06, S_NONE},
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x7E, S_ANY},  
    {0,0,0,0,OPTYPE_INDIRECT_MMN, OPTYPE_A,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x32, S_ANY},
    {0,0,0,0,OPTYPE_R, OPTYPE_R,                false, TRANSFORM_Y,      TRANSFORM_Z,    0x00, 0x40, S_NONE}, 
    {0,0,0,0,OPTYPE_HL, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2A, S_ANY},
    {0,0,0,0,OPTYPE_A, OPTYPE_INDIRECT_MMN,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3A, S_ANY}, 
    {0,0,0,0,OPTYPE_INDIRECT_MMN, OPTYPE_HL,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x22, S_ANY},
    {0,0,0,0,OPTYPE_INDIRECT_HL, OPTYPE_R,      false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x70, S_ANY},  
    {0,0,0,0,OPTYPE_A, OPTYPE_INDIRECT_DE,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x1A, S_ANY},  
    {0,0,0,0,OPTYPE_RR, OPTYPE_INDIRECT_HL,     false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x07, S_ANY},
    {0,0,0,0,OPTYPE_INDIRECT_RR, OPTYPE_A,      false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x02, S_ANY},
    {0,0,0,0,OPTYPE_IXY, OPTYPE_MMN,             true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x21, S_ANY},  
    {0,0,0,0,OPTYPE_INDIRECT_IXYd, OPTYPE_RR,    true, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x0F, S_ANY},  
    {0,0,0,0,OPTYPE_INDIRECT_MMN, OPTYPE_IXY,    true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x22, S_ANY}, 
// end optimized set
    {0,0,0,0,OPTYPE_A, OPTYPE_I,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x57, S_NONE}, 
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x7E, S_ANY},
    {0,0,0,0,OPTYPE_A, OPTYPE_MB,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6E, S_NONE}, 
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_REG_R,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x5F, S_NONE},
    {0,0,0,0,OPTYPE_A, OPTYPE_INDIRECT_BC,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x0A, S_ANY},  
    {0,0,0,0,OPTYPE_HL, OPTYPE_I,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xD7, S_NONE}, 
    {0,0,0,0,OPTYPE_INDIRECT_HL, OPTYPE_IX,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x3F, S_ANY},  
    {0,0,0,0,OPTYPE_INDIRECT_HL, OPTYPE_IY,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x3E, S_ANY},  
    {0,0,0,0,OPTYPE_INDIRECT_HL, OPTYPE_N,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x36, S_ANY},  
    {0,0,0,0,OPTYPE_INDIRECT_HL, OPTYPE_RR,     false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x0F, S_ANY}, 
    {0,0,0,0,OPTYPE_I, OPTYPE_HL,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC7, S_NONE},  
    {0,0,0,0,OPTYPE_I, OPTYPE_A,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x47, S_NONE},  
    {0,0,0,0,OPTYPE_IR, OPTYPE_IR,               true, TRANSFORM_IR3,     TRANSFORM_IR0, 0x00, 0x64, S_NONE}, 
    {0,0,0,0,OPTYPE_IR, OPTYPE_N,                true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x26, S_NONE}, 
    {0,0,0,0,OPTYPE_IR, OPTYPE_R_AEONLY,         true, TRANSFORM_IR3,     TRANSFORM_Z,   0x00, 0x60, S_NONE}, 
    {0,0,0,0,OPTYPE_IX, OPTYPE_INDIRECT_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x37, S_ANY}, 
    {0,0,0,0,OPTYPE_IY, OPTYPE_INDIRECT_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x31, S_ANY}, 
    {0,0,0,0,OPTYPE_IX, OPTYPE_INDIRECT_IXd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_ANY}, 
    {0,0,0,0,OPTYPE_IY, OPTYPE_INDIRECT_IYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_ANY}, 
    {0,0,0,0,OPTYPE_IX, OPTYPE_INDIRECT_IYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_ANY}, 
    {0,0,0,0,OPTYPE_IY, OPTYPE_INDIRECT_IXd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_ANY}, 
    {0,0,0,0,OPTYPE_IXY, OPTYPE_INDIRECT_MMN,    true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2A, S_ANY},  
    {0,0,0,0,OPTYPE_INDIRECT_IXd, OPTYPE_IX,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_ANY},
    {0,0,0,0,OPTYPE_INDIRECT_IYd, OPTYPE_IY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_ANY},
    {0,0,0,0,OPTYPE_INDIRECT_IXd, OPTYPE_IY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3E, S_ANY},
    {0,0,0,0,OPTYPE_INDIRECT_IYd, OPTYPE_IX,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3E, S_ANY},
    {0,0,0,0,OPTYPE_INDIRECT_IXYd, OPTYPE_N,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x36, S_ANY},  
    {0,0,0,0,OPTYPE_INDIRECT_IXYd, OPTYPE_R,     true, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x70, S_ANY},  
    {0,0,0,0,OPTYPE_MB, OPTYPE_A,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6D, S_NONE}, 
    {0,0,0,0,OPTYPE_INDIRECT_MMN, OPTYPE_RR,    false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x43, S_ANY},
    {0,0,0,0,OPTYPE_INDIRECT_MMN, OPTYPE_SP,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x73, S_ANY},
    {0,0,0,0,OPTYPE_REG_R, OPTYPE_A,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x4F, S_NONE},
    {0,0,0,0,OPTYPE_R, OPTYPE_INDIRECT_HL,      false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x46, S_ANY},
    {0,0,0,0,OPTYPE_R_AEONLY, OPTYPE_IR,         true, TRANSFORM_Y,      TRANSFORM_IR0,  0x00, 0x44, S_NONE},
    {0,0,0,0,OPTYPE_R, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x46, S_ANY},
    {0,0,0,0,OPTYPE_RR, OPTYPE_INDIRECT_IXYd,    true, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x07, S_ANY},
    {0,0,0,0,OPTYPE_RR, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x4B, S_ANY},
    {0,0,0,0,OPTYPE_INDIRECT_HL, OPTYPE_A,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x77, S_ANY},
    {0,0,0,0,OPTYPE_SP, OPTYPE_HL,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF9, S_ANY},
    {0,0,0,0,OPTYPE_SP, OPTYPE_IXY,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF9, S_ANY},
    {0,0,0,0,OPTYPE_SP, OPTYPE_MMN,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_ANY},
    {0,0,0,0,OPTYPE_SP, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7B, S_ANY},
};
operandlist_t operands_ldd[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA8, S_ANY},
};
operandlist_t operands_lddr[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB8, S_ANY},
};
operandlist_t operands_ldi[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA0, S_ANY},
};
operandlist_t operands_ldir[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB0, S_ANY},
};
operandlist_t operands_lea[] = {
// optimized set
    {0,0,0,0,OPTYPE_RR, OPTYPE_IXd,             false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x02, S_ANY},
// end optimized set
    {0,0,0,0,OPTYPE_IX, OPTYPE_IXd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x32, S_ANY},
    {0,0,0,0,OPTYPE_IY, OPTYPE_IXd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x55, S_ANY},
    {0,0,0,0,OPTYPE_IX, OPTYPE_IYd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x54, S_ANY},
    {0,0,0,0,OPTYPE_IY, OPTYPE_IYd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x33, S_ANY},
    {0,0,0,0,OPTYPE_RR, OPTYPE_IYd,             false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x03, S_ANY},
};
operandlist_t operands_mlt[] = {
    {RS_RR, NOREQ, RS_NONE, NOREQ, OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x4C, S_NONE},
    {R_SP, NOREQ, RS_NONE, NOREQ, OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7C, S_ANY},
};
operandlist_t operands_neg[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x44, S_NONE},
};
operandlist_t operands_nop[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x00, S_NONE},
};
operandlist_t operands_or[] = {
// optimized set
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xB0, S_NONE},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF6, S_NONE},
// end optimized set
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_ANY},
    {R_A, NOREQ, RS_IR, NOREQ, OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xB4, S_NONE},
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_ANY},
    {R_A, NOREQ, RS_NONE, NOREQ, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF6, S_NONE},
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xB0, S_NONE},
    // same set, without A register
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_ANY},
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xB4, S_NONE},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_ANY},
};
operandlist_t operands_otd2r[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBC, S_ANY},
};
operandlist_t operands_otdm[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8B, S_ANY},
};
operandlist_t operands_otdmr[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9B, S_ANY},
};
operandlist_t operands_otdr[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBB, S_ANY},
};
operandlist_t operands_otdrx[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xCB, S_ANY},
};
operandlist_t operands_oti2r[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB4, S_ANY},
};
operandlist_t operands_otim[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x83, S_ANY},
};
operandlist_t operands_otimr[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x93, S_ANY},
};
operandlist_t operands_otir[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB3, S_ANY},
};
operandlist_t operands_otirx[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC3, S_ANY},
};
operandlist_t operands_out[] = {
    {0,0,0,0,OPTYPE_INDIRECT_BC, OPTYPE_R,      false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x41, S_NONE},
    {0,0,0,0,OPTYPE_INDIRECT_C, OPTYPE_R,       false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x41, S_NONE},
    {0,0,0,0,OPTYPE_INDIRECT_N, OPTYPE_A,       false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD3, S_NONE},
}; 
operandlist_t operands_out0[] = {
    {0,0,0,0,OPTYPE_INDIRECT_N, OPTYPE_R,       false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x01, S_NONE},
};
operandlist_t operands_outd[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAB, S_ANY},
};
operandlist_t operands_outd2[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAC, S_ANY},
};
operandlist_t operands_outi[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA3, S_ANY},
};
operandlist_t operands_outi2[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA4, S_ANY},
};
operandlist_t operands_pea[] = {
    {0,0,0,0,OPTYPE_IXd, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x65, S_ANY},
    {0,0,0,0,OPTYPE_IYd, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x66, S_ANY},
};
operandlist_t operands_pop[] = {
    {RS_RR, NOREQ, RS_NONE, NOREQ, OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0xC1, S_ANY},
    {0,0,0,0,OPTYPE_AF, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF1, S_ANY},
    {RS_IXY, NOREQ, RS_NONE, NOREQ, OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE1, S_ANY},
};
operandlist_t operands_push[] = {
    {RS_RR, NOREQ, RS_NONE, NOREQ, OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0xC5, S_ANY},
    {0,0,0,0,OPTYPE_AF, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF5, S_ANY},
    {RS_IXY, NOREQ, RS_NONE, NOREQ, OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE5, S_ANY},
};
operandlist_t operands_res[] = {
    {RS_NONE, STATE_IMMEDIATE, R_HL, STATE_INDIRECT, OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x86, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_IXY, STATE_INDIRECT | STATE_DISPLACEMENT, OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x86, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_R, NOREQ, OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_BIT,    TRANSFORM_Z,    0xCB, 0x80, S_NONE},
};
operandlist_t operands_ret[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC9, S_LIL | S_LIS},
    {0,0,0,0,OPTYPE_CC, OPTYPE_NONE,            false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC0, S_LIL | S_LIS},
};
operandlist_t operands_reti[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x4D, S_LIL | S_LIS},
};
operandlist_t operands_retn[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x45, S_LIL | S_LIS},
};
operandlist_t operands_rl[] = {
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x16, S_ANY},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x16, S_ANY},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x10, S_NONE},
};
operandlist_t operands_rla[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x17, S_NONE},
};
operandlist_t operands_rlc[] = {
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x06, S_ANY},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x06, S_ANY},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,     TRANSFORM_NONE,  0xCB, 0x00, S_NONE},
};
operandlist_t operands_rlca[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x07, S_NONE},
};
operandlist_t operands_rld[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6F, S_NONE},
};
operandlist_t operands_rr[] = {
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x1E, S_ANY},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x1E, S_ANY},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x18, S_NONE},
};
operandlist_t operands_rra[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x1F, S_NONE},
};
operandlist_t operands_rrc[] = {
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x0E, S_ANY},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x0E, S_ANY},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x08, S_NONE},
};
operandlist_t operands_rrca[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x0F, S_NONE},
};
operandlist_t operands_rrd[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x67, S_NONE},
};
operandlist_t operands_rsmix[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7E, S_NONE},
};
operandlist_t operands_rst[] = {
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_N,      TRANSFORM_NONE, 0x00, 0xC7, S_ANY},
};
operandlist_t operands_sbc[] = {
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_ANY},
    {R_A, NOREQ, RS_IR, NOREQ, OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x9C, S_NONE},
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_ANY},
    {R_A, NOREQ, RS_NONE, NOREQ, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xDE, S_NONE},
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x98, S_NONE},
    // same set, without A register
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_ANY},
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0x9C, S_NONE},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xDE, S_NONE},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0x98, S_NONE},

    {R_HL, NOREQ, RS_RR, NOREQ, OPTYPE_HL,OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x42, S_ANY},
    {R_HL, NOREQ, R_SP, NOREQ, OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x72, S_ANY},
};
operandlist_t operands_scf[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_NONE},
};
operandlist_t operands_set[] = {
    {RS_NONE, STATE_IMMEDIATE, R_HL, STATE_INDIRECT, OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0xC6, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_IXY, STATE_INDIRECT | STATE_DISPLACEMENT, OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0xC6, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_R, NOREQ, OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_BIT,    TRANSFORM_Z,    0xCB, 0xC0, S_NONE},
};
operandlist_t operands_sla[] = {
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x26, S_ANY},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x26, S_ANY},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x20, S_NONE},
};
operandlist_t operands_slp[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x76, S_NONE},
};
operandlist_t operands_sra[] = {
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x2E, S_ANY},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x2E, S_ANY},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x28, S_NONE},
};
operandlist_t operands_srl[] = {
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x3E, S_ANY},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x3E, S_ANY},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x38, S_NONE},
};
operandlist_t operands_stmix[] = {
    {RS_NONE, NOREQ, RS_NONE, NOREQ, OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7D, S_NONE},
};
operandlist_t operands_sub[] = {
// optimized set
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,    false, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0x96, S_ANY},
// end optimized set
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x96, S_ANY},
    {R_A, NOREQ, RS_IR, NOREQ, OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x94, S_NONE},
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x96, S_ANY},
    {R_A, NOREQ, RS_NONE, NOREQ, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD6, S_NONE},
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x90, S_NONE},
    // same set, without A register
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,              true, TRANSFORM_IR0,    TRANSFORM_NONE,0x00, 0x94, S_NONE},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,   true, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0x96, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,              false, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0xD6, S_NONE},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,              false, TRANSFORM_Z,      TRANSFORM_NONE,0x00, 0x90, S_NONE},
};
operandlist_t operands_tst[] = {
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x34, S_ANY},
    {R_A, NOREQ, RS_NONE, NOREQ, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x64, S_NONE},
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x04, S_NONE},
    // same set, without A register
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x34, S_ANY},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x64, S_NONE},
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x04, S_NONE},
};
operandlist_t operands_tstio[] = {
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x74, S_NONE},
};
operandlist_t operands_xor[] = {
// optimized set
    {RS_R, NOREQ, RS_NONE, NOREQ, OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xA8, S_NONE},
    {RS_NONE, STATE_IMMEDIATE, RS_NONE, NOREQ, OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEE, S_NONE},
    {R_HL, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_ANY},
// end optimized set
    {R_A, NOREQ, R_HL, STATE_INDIRECT, OPTYPE_A,OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_ANY},
    {R_A, NOREQ, RS_IR, NOREQ, OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xAC, S_NONE},
    {R_A, NOREQ, RS_IXY, STATE_INDIRECT, OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_ANY},
    {R_A, NOREQ, RS_NONE, NOREQ, OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEE, S_NONE},
    {R_A, NOREQ, RS_R, NOREQ, OPTYPE_A,OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xA8, S_NONE},
    // same set, without A register
    {RS_IR, NOREQ, RS_NONE, NOREQ, OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xAC, S_NONE},
    {RS_IXY, STATE_INDIRECT, RS_NONE, NOREQ, OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_ANY},
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
