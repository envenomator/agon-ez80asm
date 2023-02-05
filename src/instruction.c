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
    return op->cc;
}
bool ir_match(operand *op) {
    return ((op->reg >= R_IXH) && (op->reg <= R_IYL));
}
bool ixy_match(operand *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && !(op->indirect)  && !(op->displacement_provided));
}
bool ixyd_match(operand *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && !(op->indirect) && (op->displacement_provided));
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
bool b_match(operand *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool af_match(operand *op) {
    return(op->reg == R_AF);
}
bool de_match(operand *op) {
    return(op->reg == R_DE);
}
bool nselect_match(operand *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool indirect_n_match(operand *op) {
    return ((op->indirect) && (op->immediate_provided));
}
bool indirect_bc_match(operand *op) {
    return((op->indirect) && (op->reg == R_BC));
}
bool indirect_c_match(operand *op) {
    return((op->indirect) && (op->reg == R_C));
}
bool indirect_ixy_match(operand *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && (op->indirect)  && !(op->displacement_provided));
}
bool cca_match(operand *op) {
    return ((op->cc) && 
            ((op->cc_index == CC_INDEX_NZ) ||
             (op->cc_index == CC_INDEX_Z) ||
             (op->cc_index == CC_INDEX_NC) ||
             (op->cc_index == CC_INDEX_C)));
}
bool indirect_de_match(operand *op) {
    return((op->indirect) && (op->reg == R_DE));
}
void none_transform(opcodetransformtype type, operand *op) {
    return;
}
void cc_transform(opcodetransformtype type, operand *op) {
    if(type == TRANSFORM_Y) {
        output.opcode |= (op->cc_index << 3);
    }
    return;
}
void ir_transform(opcodetransformtype type, operand *op) {
    if(type == TRANSFORM_DDFD) {
        switch(op->reg) {
            case R_IXH:
                output.prefix1 = 0xDD;
                return;
            case R_IXL:
                output.prefix1 = 0xDD;
                if(op->position == POS_SOURCE) output.opcode++; // bit 0
                else output.opcode |= 0x08; // bit 3
                return;
            case R_IYH:
                output.prefix1 = 0xFD;
                return;
            case R_IYL:
                output.prefix1 = 0xFD;
                if(op->position == POS_SOURCE) output.opcode++; // bit 0
                else output.opcode |= 0x08; // bit 3
                return;
            default:
                break;

        }
    }
    error(message[ERROR_TRANSFORMATION]);
    return;
}
void ixy_transform(opcodetransformtype type, operand *op) {
    if(type == TRANSFORM_DDFD) {
        switch(op->reg) {
            case R_IX:
                output.prefix1 = 0xDD;
                return;
            case R_IY:
                output.prefix1 = 0xFD;
                return;
            default:
                break;
        }
    }
    error(message[ERROR_TRANSFORMATION]);
    return;
}
void ixyd_transform(opcodetransformtype type, operand *op) {
    return;
}
void indirect_ixyd_transform(opcodetransformtype type, operand *op) {
    if(type == TRANSFORM_DDFD) {
        switch(op->reg) {
            case R_IX:
                output.prefix1 = 0xDD;
                return;
            case R_IY:
                output.prefix1 = 0xFD;
                return;
            default:
                break;
        }
    }
    error(message[ERROR_TRANSFORMATION]);
    return;
}
void mmn_transform(opcodetransformtype type, operand *op) {
    return;
}
void indirect_mmn_transform(opcodetransformtype type, operand *op) {
    return;
}
void n_transform(opcodetransformtype type, operand *op) {
    return;
}
void a_transform(opcodetransformtype type, operand *op) {
    return;
}
void hl_transform(opcodetransformtype type, operand *op) {
    return;
}
void indirect_hl_transform(opcodetransformtype type, operand *op) {
    return;
}
void rr_transform(opcodetransformtype type, operand *op) {
    switch(type) {
        case TRANSFORM_P:
            output.opcode |= (op->reg_index << 4);
            return;
        default:
            error(message[ERROR_TRANSFORMATION]);
    }
    return;
}
void indirect_rr_transform(opcodetransformtype type, operand *op) {
    return;
}
void rxy_transform(opcodetransformtype type, operand *op) {
    return;
}
void sp_transform(opcodetransformtype type, operand *op) {
    return;
}
void indirect_sp_transform(opcodetransformtype type, operand *op) {
    return;
}
void r_transform(opcodetransformtype type, operand *op) {
    switch(type) {
        case TRANSFORM_Z:
            output.opcode |= op->reg_index;
            return;
        case TRANSFORM_Y:
            output.opcode |= (op->reg_index << 3);
            return;
        default:
            error(message[ERROR_TRANSFORMATION]);
    }
    return;
}
void reg_r_transform(opcodetransformtype type, operand *op) {
    return;
}
void mb_transform(opcodetransformtype type, operand *op) {
    return;
}
void i_transform(opcodetransformtype type, operand *op) {
    return;
}
void b_transform(opcodetransformtype type, operand *op) {
    switch(type) {
        case TRANSFORM_Y:
            output.opcode |= (op->immediate << 3);
            return;
        default:
            error(message[ERROR_TRANSFORMATION]);
    }
    return;
}
void af_transform(opcodetransformtype type, operand *op) {
    return;
}
void de_transform(opcodetransformtype type, operand *op) {
    return;
}
void nselect_transform(opcodetransformtype type, operand *op) {
    uint8_t y;
    if(type == TRANSFORM_P) {
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
                error(message[ERROR_INVALIDOPERAND]);
                y = 0;
        }
        output.opcode |= (y << 3); // shift 3 lower bits 3 to the left
        op->immediate_provided = false; // no separate output for this transform
    }
    return;
}
void indirect_n_transform(opcodetransformtype type, operand *op) {
    return;
}
void indirect_bc_transform(opcodetransformtype type, operand *op) {
    return;
}
void indirect_c_transform(opcodetransformtype type, operand *op) {
    return;
}
void indirect_ixy_transform(opcodetransformtype type, operand *op) {
    if(type == TRANSFORM_DDFD) {
        switch(op->reg) {
            case R_IX:
                output.prefix1 = 0xDD;
                return;
            case R_IY:
                output.prefix1 = 0xFD;
                return;
            default:
                break;
        }
    }
    error(message[ERROR_TRANSFORMATION]);
    return;
}
void cca_transform(opcodetransformtype type, operand *op) {
    if(type == TRANSFORM_P) {
        output.opcode |= (op->cc_index << 3);
    }
    return;
}
void indirect_de_transform(opcodetransformtype type, operand *op) {
    return;
}

instruction * instruction_table[INSTRUCTION_TABLE_SIZE]; // hashtable of possible instructions, indexed by mnemonic name
operandtype_match operandtype_matchlist[] = {            // table with fast access to functions that perform matching to an specific operandtype
    {OPTYPE_NONE, none_match, none_transform},
    {OPTYPE_CC, cc_match, cc_transform},
    {OPTYPE_IR, ir_match, ir_transform},
    {OPTYPE_IXY, ixy_match, ixy_transform},
    {OPTYPE_IXYd, ixyd_match, ixyd_transform},
    {OPTYPE_INDIRECT_IXYd, indirect_ixyd_match, indirect_ixyd_transform},
    {OPTYPE_MMN, mmn_match, mmn_transform},
    {OPTYPE_INDIRECT_MMN, indirect_mmn_match, indirect_mmn_transform},
    {OPTYPE_N, n_match, n_transform},
    {OPTYPE_A, a_match, a_transform},
    {OPTYPE_HL, hl_match, hl_transform},
    {OPTYPE_INDIRECT_HL, indirect_hl_match, indirect_hl_transform},
    {OPTYPE_RR, rr_match, rr_transform},
    {OPTYPE_INDIRECT_RR, indirect_rr_match, indirect_rr_transform},
    {OPTYPE_RXY, rxy_match, rxy_transform},
    {OPTYPE_SP, sp_match, sp_transform},
    {OPTYPE_INDIRECT_SP, indirect_sp_match, indirect_sp_transform},
    {OPTYPE_R, r_match, r_transform},
    {OPTYPE_REG_R, reg_r_match, reg_r_transform},
    {OPTYPE_MB, mb_match, mb_transform},
    {OPTYPE_I, i_match, i_transform},
    {OPTYPE_BIT, b_match, b_transform},
    {OPTYPE_AF, af_match, af_transform},
    {OPTYPE_DE, de_match, de_transform},
    {OPTYPE_NSELECT, nselect_match, nselect_transform},
    {OPTYPE_INDIRECT_N, indirect_n_match, indirect_n_transform},
    {OPTYPE_INDIRECT_BC, indirect_bc_match, indirect_bc_transform},
    {OPTYPE_INDIRECT_C, indirect_c_match, indirect_c_transform},
    {OPTYPE_INDIRECT_IXY, indirect_ixy_match, indirect_ixy_transform},
    {OPTYPE_CCA, cca_match, cca_transform},
    {OPTYPE_INDIRECT_DE, indirect_de_match, indirect_de_transform},
};

unsigned int collisions;    // internal use

operandlist operands_adc[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,  TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x8E, SL_ONLY}, // tested
    {OPTYPE_A, OPTYPE_IR,           TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0x8C, NONE}, // tested
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0x8E, SL_ONLY}, // tested
    {OPTYPE_A, OPTYPE_N,            TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0xCE, NONE}, // tested
    {OPTYPE_A, OPTYPE_R,            TRANSFORM_NONE, TRANSFORM_Z,    0x00, 0x88, NONE}, // tested
    {OPTYPE_HL, OPTYPE_RR,          TRANSFORM_NONE, TRANSFORM_P,    0xED, 0x4A, SL_ONLY}, // tested
    {OPTYPE_HL, OPTYPE_SP,          TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0x7A, SL_ONLY}, // tested
};
operandlist operands_add[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,  TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x86, SL_ONLY}, // tested
    {OPTYPE_A, OPTYPE_IR,           TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0x84, NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0x86, SL_ONLY},
    {OPTYPE_A, OPTYPE_N,            TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0xC6, NONE},
    {OPTYPE_A, OPTYPE_R,            TRANSFORM_NONE, TRANSFORM_Z,    0x00, 0x80, NONE},
    {OPTYPE_A, OPTYPE_RR,           TRANSFORM_NONE, TRANSFORM_P,    0x00, 0x09, SL_ONLY},
    {OPTYPE_A, OPTYPE_SP,           TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x39, SL_ONLY},
    {OPTYPE_IXY, OPTYPE_RXY,        TRANSFORM_DDFD, TRANSFORM_P,    0x00, 0x09, SL_ONLY}, // zeker testen
    {OPTYPE_IXY, OPTYPE_SP,         TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x39, SL_ONLY}, // zeker testen
};
operandlist operands_and[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,  TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0xA6, SL_ONLY},
    {OPTYPE_A, OPTYPE_IR,           TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0xA4, NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0xA6, SL_ONLY},
    {OPTYPE_A, OPTYPE_N,            TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0xE6, NONE},
    {OPTYPE_A, OPTYPE_R,            TRANSFORM_NONE, TRANSFORM_Z,    0x00, 0xA0, NONE},
};
operandlist operands_bit[] = {
    {OPTYPE_BIT, OPTYPE_INDIRECT_HL,  TRANSFORM_Y, TRANSFORM_NONE,  0xCB, 0x46, SL_ONLY}, // tested
    {OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,TRANSFORM_Y, TRANSFORM_DDFD,  0xCB, 0x46, SL_ONLY}, // tested
    {OPTYPE_BIT, OPTYPE_R,            TRANSFORM_Y, TRANSFORM_Z,     0xCB, 0x40, NONE},
};
operandlist operands_call[] = {
    {OPTYPE_CC, OPTYPE_MMN,         TRANSFORM_Y, TRANSFORM_NONE,   0x00, 0xC4, ANY},  // tested
    {OPTYPE_MMN, OPTYPE_NONE,       TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xCD, ANY}, // tested
};
operandlist operands_ccf[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0x3F, NONE},
};
operandlist operands_cp[]= {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,  TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xBE, SL_ONLY},
    {OPTYPE_A, OPTYPE_IR,           TRANSFORM_NONE, TRANSFORM_DDFD,0x00, 0xBC, NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,TRANSFORM_NONE, TRANSFORM_DDFD,0x00, 0xBE, SL_ONLY},
    {OPTYPE_A, OPTYPE_N,            TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xFE, NONE},
    {OPTYPE_A, OPTYPE_R,            TRANSFORM_NONE, TRANSFORM_Z,   0x00, 0xB8, NONE},
};
operandlist operands_cpd[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xA9, SL_ONLY},
};
operandlist operands_cpdr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xB9, SL_ONLY},
};
operandlist operands_cpi[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xA1, SL_ONLY},
};
operandlist operands_cpir[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xB1, SL_ONLY},
};
operandlist operands_cpl[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0x2F, NONE},
};
operandlist operands_daa[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0x27, NONE},
};
operandlist operands_dec[]= {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0x35, SL_ONLY}, // tested
    {OPTYPE_IR, OPTYPE_NONE,      TRANSFORM_DDFD, TRANSFORM_NONE,0x00, 0x25, NONE}, // tested
    {OPTYPE_IXY, OPTYPE_NONE,     TRANSFORM_DDFD, TRANSFORM_NONE,0x00, 0x2B, SL_ONLY}, // tested
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE, TRANSFORM_DDFD, TRANSFORM_NONE,0x00,0x35,SL_ONLY}, // tested
    {OPTYPE_R, OPTYPE_NONE,       TRANSFORM_Y, TRANSFORM_NONE, 0x00, 0x05, NONE}, // tested
    {OPTYPE_RR, OPTYPE_NONE,      TRANSFORM_P, TRANSFORM_NONE, 0x00, 0x0B, SL_ONLY}, // tested
    {OPTYPE_SP, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x3B, SL_ONLY}, // tested
};
operandlist operands_di[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xF3, NONE},
};
operandlist operands_djnz[]= {
    {OPTYPE_N, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0x10, NONE},
};
operandlist operands_ei[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xFB, NONE},
};
operandlist operands_ex[]= {
    {OPTYPE_AF, OPTYPE_AF,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0x08, NONE},
    {OPTYPE_DE, OPTYPE_HL,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xEB, NONE},
    {OPTYPE_INDIRECT_SP, OPTYPE_HL, TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0xE3, SL_ONLY},
    {OPTYPE_INDIRECT_SP, OPTYPE_IXY, TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0xE3, SL_ONLY},
};
operandlist operands_exx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xD9, NONE},
};
operandlist operands_halt[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x76, NONE},
};
operandlist operands_im[]= {
    {OPTYPE_NSELECT, OPTYPE_NONE,      TRANSFORM_P, TRANSFORM_NONE,0xED, 0x46, NONE}, // tested
};
operandlist operands_in[]= {
    {OPTYPE_A, OPTYPE_INDIRECT_N,      TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xDB, NONE}, // tested
    {OPTYPE_R, OPTYPE_INDIRECT_BC,     TRANSFORM_Y, TRANSFORM_NONE, 0xED, 0x40, NONE}, // tested
    {OPTYPE_R, OPTYPE_INDIRECT_C,      TRANSFORM_Y, TRANSFORM_NONE, 0xED, 0x40, NONE}, // tested
};
operandlist operands_in0[]= {
    {OPTYPE_R, OPTYPE_INDIRECT_N,     TRANSFORM_Y, TRANSFORM_NONE, 0xED, 0x00, NONE}, // tested
};
operandlist operands_inc[]= {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,       TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x34, SL_ONLY}, // tested
    {OPTYPE_IR, OPTYPE_NONE,                TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x24, NONE}, // tested
    {OPTYPE_IXY, OPTYPE_NONE,               TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x23, SL_ONLY}, // tested
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,     TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x34, SL_ONLY}, // tested
    {OPTYPE_R, OPTYPE_NONE,                 TRANSFORM_Y, TRANSFORM_NONE, 0x00, 0x04, NONE}, // tested
    {OPTYPE_RR, OPTYPE_NONE,                TRANSFORM_P, TRANSFORM_NONE, 0x00, 0x03, SL_ONLY}, // tested
    {OPTYPE_SP, OPTYPE_NONE,                TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x33, SL_ONLY}, // tested
};
operandlist operands_ind[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xAA, SL_ONLY}, // tested
};
operandlist operands_ind2[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x8C, SL_ONLY}, // tested
};
operandlist operands_ind2r[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x9C, SL_ONLY}, // tested
};
operandlist operands_indm[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x8A, SL_ONLY}, // tested
};
operandlist operands_indmr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x9A, SL_ONLY}, // tested
};
operandlist operands_indr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xBA, SL_ONLY}, // tested
};
operandlist operands_indrx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xCA, SL_ONLY}, // tested
};
operandlist operands_ini[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xA2, SL_ONLY}, // tested
};
operandlist operands_ini2[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x84, SL_ONLY}, // tested
};
operandlist operands_ini2r[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x94, SL_ONLY}, // tested
};
operandlist operands_inim[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x82, SL_ONLY}, // tested
};
operandlist operands_inimr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0x92, SL_ONLY}, // tested
};
operandlist operands_inir[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xB2, SL_ONLY}, // tested
};
operandlist operands_inirx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,      TRANSFORM_NONE, TRANSFORM_NONE,0xED, 0xC2, SL_ONLY}, // tested
};
operandlist operands_jp[] = {
    {OPTYPE_CC, OPTYPE_MMN,         TRANSFORM_Y, TRANSFORM_NONE,   0x00, 0xC2, ANY},
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE, TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0xE9, SL_ONLY},
    {OPTYPE_INDIRECT_IXY, OPTYPE_NONE, TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0xE9, SL_ONLY},
    {OPTYPE_MMN, OPTYPE_NONE,       TRANSFORM_NONE, TRANSFORM_NONE,0x00, 0xC3, SISLIL},
};
operandlist operands_jr[]= {
    {OPTYPE_CCA, OPTYPE_N,          TRANSFORM_P, TRANSFORM_NONE, 0x00, 0x20, NONE}, // tested without negative numbers
    {OPTYPE_N, OPTYPE_NONE,         TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x18, NONE},
};
operandlist operands_ld[] = {
    {OPTYPE_A, OPTYPE_I,              TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0x57, NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,  TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0x7E, SL_ONLY},
    {OPTYPE_A, OPTYPE_MB,             TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0x6E, NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_MMN,   TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x3A, SISLIL},
    {OPTYPE_A, OPTYPE_REG_R,          TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0x5F, NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_BC,    TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x0A, SL_ONLY},
    {OPTYPE_A, OPTYPE_INDIRECT_DE,    TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x1A, SL_ONLY},
    {OPTYPE_A, OPTYPE_INDIRECT_HL,    TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x7E, SL_ONLY},
    {OPTYPE_HL, OPTYPE_I,             TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0xD7, NONE},
    {OPTYPE_INDIRECT_HL, OPTYPE_IXY,  TRANSFORM_NONE, TRANSFORM_DDFD, 0xED, 0x3F, SL_ONLY},
    {OPTYPE_INDIRECT_HL, OPTYPE_N,    TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x36, SL_ONLY},
    {OPTYPE_INDIRECT_HL, OPTYPE_R,    TRANSFORM_NONE, TRANSFORM_Z,    0x00, 0x70, SL_ONLY},
    {OPTYPE_INDIRECT_HL, OPTYPE_RR,   TRANSFORM_NONE, TRANSFORM_P,    0xED, 0x0F, SL_ONLY},
    {OPTYPE_I, OPTYPE_HL,             TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0xC7, NONE},
    {OPTYPE_I, OPTYPE_A,              TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0x47, NONE},
    {OPTYPE_IR, OPTYPE_IR,            TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0x64, NONE}, // CHECKEN!!
    {OPTYPE_IR, OPTYPE_N,             TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x26, NONE}, // CHECKEN
    {OPTYPE_IR, OPTYPE_R,             TRANSFORM_DDFD, TRANSFORM_Z,    0x00, 0x60, NONE}, // CHECKEN!!!
    {OPTYPE_IXY, OPTYPE_INDIRECT_HL,  TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x37, SL_ONLY}, // CHECKEN niet ok
    {OPTYPE_IXY, OPTYPE_INDIRECT_IXYd,TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x37, SL_ONLY}, // CHECKEN niet ok
    {OPTYPE_IXY, OPTYPE_MMN,          TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x21, SISLIL},
    {OPTYPE_IXY, OPTYPE_INDIRECT_MMN, TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x2A, SISLIL},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_IXY,TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x3F, SL_ONLY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_N,  TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x36, SL_ONLY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_R,  TRANSFORM_DDFD, TRANSFORM_NONE, 0x00, 0x70, SL_ONLY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_RR, TRANSFORM_DDFD, TRANSFORM_P,    0x00, 0x70, SL_ONLY},
    {OPTYPE_MB, OPTYPE_A,             TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0x6D, NONE},
    {OPTYPE_INDIRECT_MMN, OPTYPE_A,   TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x32, ANY},
    {OPTYPE_INDIRECT_MMN, OPTYPE_IXY, TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0x22, SISLIL},
    {OPTYPE_INDIRECT_MMN, OPTYPE_RR,  TRANSFORM_NONE, TRANSFORM_P,    0x00, 0x22, SISLIL},
    {OPTYPE_INDIRECT_MMN, OPTYPE_SP,  TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x73, SISLIL},
    {OPTYPE_REG_R, OPTYPE_A,          TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0x4F, NONE},
    {OPTYPE_R, OPTYPE_INDIRECT_HL,    TRANSFORM_Y,    TRANSFORM_NONE, 0x00, 0x46, NONE},
    {OPTYPE_R, OPTYPE_IR,             TRANSFORM_Y,    TRANSFORM_DDFD, 0x00, 0x44, NONE}, // CHECKEN!!
    {OPTYPE_R, OPTYPE_INDIRECT_IXYd,  TRANSFORM_Y,    TRANSFORM_DDFD, 0x00, 0x46, NONE}, // CHECKEN!!
    {OPTYPE_R, OPTYPE_N,              TRANSFORM_Y,    TRANSFORM_NONE, 0x00, 0x06, NONE},
    {OPTYPE_R, OPTYPE_R,              TRANSFORM_Y,    TRANSFORM_Z,    0x00, 0x80, NONE}, // tested
    {OPTYPE_RR, OPTYPE_INDIRECT_HL,   TRANSFORM_P,    TRANSFORM_NONE, 0xED, 0x07, SL_ONLY},
    {OPTYPE_RR, OPTYPE_INDIRECT_IXYd, TRANSFORM_P,    TRANSFORM_DDFD, 0x00, 0x07, SL_ONLY},
    {OPTYPE_RR, OPTYPE_MMN,           TRANSFORM_P,    TRANSFORM_NONE, 0x00, 0x01, SISLIL},
    {OPTYPE_HL, OPTYPE_INDIRECT_MMN,  TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x2A, SISLIL},
    {OPTYPE_RR, OPTYPE_INDIRECT_MMN,  TRANSFORM_P,    TRANSFORM_NONE, 0xED, 0x4B, SISLIL},
    {OPTYPE_INDIRECT_HL, OPTYPE_A,    TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x77, SL_ONLY},
    {OPTYPE_INDIRECT_RR, OPTYPE_A,    TRANSFORM_P,    TRANSFORM_NONE, 0x00, 0x02, SL_ONLY},
    {OPTYPE_SP, OPTYPE_HL,            TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0xF9, SL_ONLY},
    {OPTYPE_SP, OPTYPE_IXY,           TRANSFORM_NONE, TRANSFORM_DDFD, 0x00, 0xF9, SL_ONLY},
    {OPTYPE_SP, OPTYPE_MMN,           TRANSFORM_NONE, TRANSFORM_NONE, 0x00, 0x31, SL_ONLY},
    {OPTYPE_SP, OPTYPE_INDIRECT_MMN,  TRANSFORM_NONE, TRANSFORM_NONE, 0xED, 0x7B, SISLIL},
};

operandlist operands_test[] = {
    {OPTYPE_R, OPTYPE_R,            TRANSFORM_Y, TRANSFORM_Z, 0x00, 0x80, NONE},
};

instruction instructions[] = {
    {"test",EZ80, sizeof(operands_test)/sizeof(operandlist),operands_test},
    {"adc", EZ80, sizeof(operands_adc)/sizeof(operandlist), operands_adc},
    {"add", EZ80, sizeof(operands_add)/sizeof(operandlist), operands_add},
    {"and", EZ80, sizeof(operands_and)/sizeof(operandlist), operands_and},
    {"bit", EZ80, sizeof(operands_bit)/sizeof(operandlist), operands_bit},
    {"call",EZ80, sizeof(operands_call)/sizeof(operandlist), operands_call},
    {"ccf",EZ80, sizeof(operands_ccf)/sizeof(operandlist), operands_ccf},
    {"cp",EZ80, sizeof(operands_cp)/sizeof(operandlist), operands_cp},
    {"cpd",EZ80, sizeof(operands_cpd)/sizeof(operandlist), operands_cpd},
    {"cpdr",EZ80, sizeof(operands_cpdr)/sizeof(operandlist), operands_cpdr},
    {"cpi",EZ80, sizeof(operands_cpi)/sizeof(operandlist), operands_cpi},
    {"cpir",EZ80, sizeof(operands_cpir)/sizeof(operandlist), operands_cpir},
    {"cpl",EZ80, sizeof(operands_cpl)/sizeof(operandlist), operands_cpl},
    {"daa",EZ80, sizeof(operands_daa)/sizeof(operandlist), operands_daa},
    {"dec",EZ80, sizeof(operands_dec)/sizeof(operandlist), operands_dec},
    {"di",EZ80, sizeof(operands_di)/sizeof(operandlist), operands_di},
    {"djnz",EZ80, sizeof(operands_djnz)/sizeof(operandlist), operands_djnz},
    {"ei",EZ80, sizeof(operands_ei)/sizeof(operandlist), operands_ei},
    {"ex",EZ80, sizeof(operands_ex)/sizeof(operandlist), operands_ex},
    {"exx",EZ80, sizeof(operands_exx)/sizeof(operandlist), operands_exx},
    {"halt",EZ80, sizeof(operands_halt)/sizeof(operandlist), operands_halt},
    {"im",EZ80, sizeof(operands_im)/sizeof(operandlist), operands_im},
    {"in",EZ80, sizeof(operands_in)/sizeof(operandlist), operands_in},
    {"in0",EZ80, sizeof(operands_in0)/sizeof(operandlist), operands_in0},
    {"inc",EZ80, sizeof(operands_inc)/sizeof(operandlist), operands_inc},
    {"ind",EZ80, sizeof(operands_ind)/sizeof(operandlist), operands_ind},
    {"ind2",EZ80, sizeof(operands_ind2)/sizeof(operandlist), operands_ind2},
    {"ind2r",EZ80, sizeof(operands_ind2r)/sizeof(operandlist), operands_ind2r},
    {"indm",EZ80, sizeof(operands_indm)/sizeof(operandlist), operands_indm},
    {"indmr",EZ80, sizeof(operands_indmr)/sizeof(operandlist), operands_indmr},
    {"indr",EZ80, sizeof(operands_indr)/sizeof(operandlist), operands_indr},
    {"indrx",EZ80, sizeof(operands_indrx)/sizeof(operandlist), operands_indrx},
    {"ini",EZ80, sizeof(operands_ini)/sizeof(operandlist), operands_ini},
    {"ini2",EZ80, sizeof(operands_ini2)/sizeof(operandlist), operands_ini2},
    {"ini2r",EZ80, sizeof(operands_ini2r)/sizeof(operandlist), operands_ini2r},
    {"inim",EZ80, sizeof(operands_inim)/sizeof(operandlist), operands_inim},
    {"inimr",EZ80, sizeof(operands_inimr)/sizeof(operandlist), operands_inimr},
    {"inir",EZ80, sizeof(operands_inir)/sizeof(operandlist), operands_inir},
    {"inirx",EZ80, sizeof(operands_inirx)/sizeof(operandlist), operands_inirx},
    {"jp",  EZ80, sizeof(operands_jp)/sizeof(operandlist), operands_jp},
    {"jr",  EZ80, sizeof(operands_jr)/sizeof(operandlist), operands_jr},
    {"ld",  EZ80, sizeof(operands_ld)/sizeof(operandlist), operands_ld},
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
    {.name = "nop", .function = nOPTYPE_action},
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