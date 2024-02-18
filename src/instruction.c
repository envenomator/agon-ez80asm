#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "instruction.h"
#include "assemble.h"
#include "utils.h"
#include "globals.h"
#include "./stdint.h"

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
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_ANY},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x8C, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xCE, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x88, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_ANY},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0x8C, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_ANY},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xCE, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0x88, S_NONE},

    {OPTYPE_HL, OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x4A, S_ANY},
    {OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7A, S_ANY},
};
operandlist_t operands_add[] = {
// optimized set
    {OPTYPE_HL, OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x09, S_ANY},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x80, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_ANY},
// end optimized set
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x84, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC6, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_ANY},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0x84, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_ANY},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC6, S_NONE},

    {OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x39, S_ANY},
    {OPTYPE_IXY, OPTYPE_RXY,             true, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x09, S_ANY},
    {OPTYPE_IXY, OPTYPE_SP,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x39, S_ANY},
};
operandlist_t operands_and[] = {
// optimized set
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xA0, S_NONE},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE6, S_NONE},
// end optimized set
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_ANY},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xA4, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE6, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xA0, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_ANY},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xA4, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_ANY},
};
operandlist_t operands_bit[] = {
    {OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x46, S_ANY},
    {OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x46, S_ANY},
    {OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_Y,      TRANSFORM_Z,    0xCB, 0x40, S_NONE},
};
operandlist_t operands_call[] = {
    {OPTYPE_MMN, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xCD, S_ANY},
    {OPTYPE_CC, OPTYPE_MMN,             false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC4, S_ANY},
};
operandlist_t operands_ccf[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_NONE},
};
operandlist_t operands_cp[]= {
// optimized set
    {OPTYPE_N,  OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFE, S_NONE},
    {OPTYPE_R,  OPTYPE_NONE,            false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xB8, S_NONE},
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFE, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xB8, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_ANY},
// end optimized set
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xBC, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_ANY},
    // same set, without A register
    {OPTYPE_IR,  OPTYPE_NONE,            true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xBC, S_NONE},
    {OPTYPE_INDIRECT_IXYd,  OPTYPE_NONE, true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_ANY},
};
operandlist_t operands_cpd[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA9, S_ANY},
};
operandlist_t operands_cpdr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB9, S_ANY},
};
operandlist_t operands_cpi[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA1, S_ANY},
};
operandlist_t operands_cpir[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB1, S_ANY},
};
operandlist_t operands_cpl[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2F, S_NONE},
};
operandlist_t operands_daa[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x27, S_NONE},
};
operandlist_t operands_dec[]= {
// optimized set
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x0B, S_ANY}, 
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x05, S_NONE}, 
// end optimized set
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x35, S_ANY}, 
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x25, S_NONE}, 
    {OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2B, S_ANY}, 
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x35, S_ANY}, 
    {OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3B, S_ANY}, 
};
operandlist_t operands_di[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF3, S_NONE},
};
operandlist_t operands_djnz[]= {
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_REL,    TRANSFORM_NONE, 0x00, 0x10, S_NONE},
};
operandlist_t operands_ei[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFB, S_NONE},
};
operandlist_t operands_ex[]= {
// optimized set
    {OPTYPE_DE, OPTYPE_HL,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEB, S_NONE},
// end optimized set
    {OPTYPE_AF, OPTYPE_AF,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x08, S_NONE},
    {OPTYPE_INDIRECT_SP, OPTYPE_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE3, S_ANY},
    {OPTYPE_INDIRECT_SP, OPTYPE_IXY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE3, S_ANY},
};
operandlist_t operands_exx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD9, S_NONE},
};
operandlist_t operands_halt[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x76, S_NONE},
};
operandlist_t operands_im[]= {
    {OPTYPE_NSELECT, OPTYPE_NONE,       false, TRANSFORM_SELECT, TRANSFORM_NONE, 0xED, 0x46, S_NONE}, 
};
operandlist_t operands_in[]= {
    {OPTYPE_A, OPTYPE_INDIRECT_N,       false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xDB, S_NONE}, 
    {OPTYPE_R, OPTYPE_INDIRECT_BC,      false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x40, S_NONE}, 
    {OPTYPE_R, OPTYPE_INDIRECT_C,       false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x40, S_NONE}, 
};
operandlist_t operands_in0[]= {
    {OPTYPE_R, OPTYPE_INDIRECT_N,       false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x00, S_NONE}, 
};
operandlist_t operands_inc[]= {
// optimized set
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x03, S_ANY}, 
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x04, S_NONE}, 
    {OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x23, S_ANY}, 
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x34, S_ANY}, 
// end optimized set
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x24, S_NONE}, 
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x34, S_ANY}, 
    {OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x33, S_ANY}, 
};
operandlist_t operands_ind[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAA, S_ANY}, 
};
operandlist_t operands_ind2[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8C, S_ANY}, 
};
operandlist_t operands_ind2r[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9C, S_ANY}, 
};
operandlist_t operands_indm[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8A, S_ANY}, 
};
operandlist_t operands_indmr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9A, S_ANY}, 
};
operandlist_t operands_indr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBA, S_ANY}, 
};
operandlist_t operands_indrx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xCA, S_ANY}, 
};
operandlist_t operands_ini[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA2, S_ANY}, 
};
operandlist_t operands_ini2[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x84, S_ANY}, 
};
operandlist_t operands_ini2r[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x94, S_ANY}, 
};
operandlist_t operands_inim[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x82, S_ANY}, 
};
operandlist_t operands_inimr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x92, S_ANY}, 
};
operandlist_t operands_inir[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB2, S_ANY}, 
};
operandlist_t operands_inirx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC2, S_ANY}, 
};
operandlist_t operands_jp[] = {
    {OPTYPE_CC, OPTYPE_MMN,             false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC2, S_SISLIL},
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE9, S_ANY},
    {OPTYPE_INDIRECT_IXY, OPTYPE_NONE,   true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE9, S_SISLIL},
    {OPTYPE_MMN, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC3, S_SISLIL},
};
operandlist_t operands_jr[]= {
    {OPTYPE_CCA, OPTYPE_N,              false, TRANSFORM_CC,     TRANSFORM_REL,  0x00, 0x20, S_NONE},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_REL,    TRANSFORM_NONE, 0x00, 0x18, S_NONE},
};
operandlist_t operands_ld[] = {
// start optimized set
    {OPTYPE_RR, OPTYPE_MMN,              true, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x01, S_ANY},
    {OPTYPE_R, OPTYPE_N,                false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x06, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x7E, S_ANY},  
    {OPTYPE_INDIRECT_MMN, OPTYPE_A,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x32, S_ANY},
    {OPTYPE_R, OPTYPE_R,                false, TRANSFORM_Y,      TRANSFORM_Z,    0x00, 0x40, S_NONE}, 
    {OPTYPE_HL, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2A, S_ANY},
    {OPTYPE_A, OPTYPE_INDIRECT_MMN,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3A, S_ANY}, 
    {OPTYPE_INDIRECT_MMN, OPTYPE_HL,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x22, S_ANY},
    {OPTYPE_INDIRECT_HL, OPTYPE_R,      false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x70, S_ANY},  
    {OPTYPE_A, OPTYPE_INDIRECT_DE,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x1A, S_ANY},  
    {OPTYPE_RR, OPTYPE_INDIRECT_HL,     false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x07, S_ANY},
    {OPTYPE_INDIRECT_RR, OPTYPE_A,      false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x02, S_ANY},
    {OPTYPE_IXY, OPTYPE_MMN,             true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x21, S_ANY},  
    {OPTYPE_INDIRECT_IXYd, OPTYPE_RR,    true, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x0F, S_ANY},  
    {OPTYPE_INDIRECT_MMN, OPTYPE_IXY,    true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x22, S_ANY}, 
// end optimized set
    {OPTYPE_A, OPTYPE_I,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x57, S_NONE}, 
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x7E, S_ANY},
    {OPTYPE_A, OPTYPE_MB,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6E, S_NONE}, 
    {OPTYPE_A, OPTYPE_REG_R,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x5F, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_BC,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x0A, S_ANY},  
    {OPTYPE_HL, OPTYPE_I,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xD7, S_NONE}, 
    {OPTYPE_INDIRECT_HL, OPTYPE_IX,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x3F, S_ANY},  
    {OPTYPE_INDIRECT_HL, OPTYPE_IY,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x3E, S_ANY},  
    {OPTYPE_INDIRECT_HL, OPTYPE_N,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x36, S_ANY},  
    {OPTYPE_INDIRECT_HL, OPTYPE_RR,     false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x0F, S_ANY}, 
    {OPTYPE_I, OPTYPE_HL,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC7, S_NONE},  
    {OPTYPE_I, OPTYPE_A,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x47, S_NONE},  
    {OPTYPE_IR, OPTYPE_IR,               true, TRANSFORM_IR3,     TRANSFORM_IR0, 0x00, 0x64, S_NONE}, 
    {OPTYPE_IR, OPTYPE_N,                true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x26, S_NONE}, 
    {OPTYPE_IR, OPTYPE_R_AEONLY,         true, TRANSFORM_IR3,     TRANSFORM_Z,   0x00, 0x60, S_NONE}, 
    {OPTYPE_IX, OPTYPE_INDIRECT_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x37, S_ANY}, 
    {OPTYPE_IY, OPTYPE_INDIRECT_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x31, S_ANY}, 
    {OPTYPE_IX, OPTYPE_INDIRECT_IXd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_ANY}, 
    {OPTYPE_IY, OPTYPE_INDIRECT_IYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_ANY}, 
    {OPTYPE_IX, OPTYPE_INDIRECT_IYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_ANY}, 
    {OPTYPE_IY, OPTYPE_INDIRECT_IXd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_ANY}, 
    {OPTYPE_IXY, OPTYPE_INDIRECT_MMN,    true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2A, S_ANY},  
    {OPTYPE_INDIRECT_IXd, OPTYPE_IX,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_ANY},
    {OPTYPE_INDIRECT_IYd, OPTYPE_IY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_ANY},
    {OPTYPE_INDIRECT_IXd, OPTYPE_IY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3E, S_ANY},
    {OPTYPE_INDIRECT_IYd, OPTYPE_IX,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3E, S_ANY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_N,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x36, S_ANY},  
    {OPTYPE_INDIRECT_IXYd, OPTYPE_R,     true, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x70, S_ANY},  
    {OPTYPE_MB, OPTYPE_A,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6D, S_NONE}, 
    {OPTYPE_INDIRECT_MMN, OPTYPE_RR,    false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x43, S_ANY},
    {OPTYPE_INDIRECT_MMN, OPTYPE_SP,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x73, S_ANY},
    {OPTYPE_REG_R, OPTYPE_A,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x4F, S_NONE},
    {OPTYPE_R, OPTYPE_INDIRECT_HL,      false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x46, S_ANY},
    {OPTYPE_R_AEONLY, OPTYPE_IR,         true, TRANSFORM_Y,      TRANSFORM_IR0,  0x00, 0x44, S_NONE},
    {OPTYPE_R, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x46, S_ANY},
    {OPTYPE_RR, OPTYPE_INDIRECT_IXYd,    true, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x07, S_ANY},
    {OPTYPE_RR, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x4B, S_ANY},
    {OPTYPE_INDIRECT_HL, OPTYPE_A,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x77, S_ANY},
    {OPTYPE_SP, OPTYPE_HL,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF9, S_ANY},
    {OPTYPE_SP, OPTYPE_IXY,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF9, S_ANY},
    {OPTYPE_SP, OPTYPE_MMN,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_ANY},
    {OPTYPE_SP, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7B, S_ANY},
};
operandlist_t operands_ldd[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA8, S_ANY},
};
operandlist_t operands_lddr[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB8, S_ANY},
};
operandlist_t operands_ldi[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA0, S_ANY},
};
operandlist_t operands_ldir[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB0, S_ANY},
};
operandlist_t operands_lea[] = {
// optimized set
    {OPTYPE_RR, OPTYPE_IXd,             false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x02, S_ANY},
// end optimized set
    {OPTYPE_IX, OPTYPE_IXd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x32, S_ANY},
    {OPTYPE_IY, OPTYPE_IXd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x55, S_ANY},
    {OPTYPE_IX, OPTYPE_IYd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x54, S_ANY},
    {OPTYPE_IY, OPTYPE_IYd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x33, S_ANY},
    {OPTYPE_RR, OPTYPE_IYd,             false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x03, S_ANY},
};
operandlist_t operands_mlt[] = {
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x4C, S_NONE},
    {OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7C, S_ANY},
};
operandlist_t operands_neg[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x44, S_NONE},
};
operandlist_t operands_nop[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x00, S_NONE},
};
operandlist_t operands_or[] = {
// optimized set
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xB0, S_NONE},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF6, S_NONE},
// end optimized set
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_ANY},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xB4, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF6, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xB0, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_ANY},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xB4, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_ANY},
};
operandlist_t operands_otd2r[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBC, S_ANY},
};
operandlist_t operands_otdm[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8B, S_ANY},
};
operandlist_t operands_otdmr[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9B, S_ANY},
};
operandlist_t operands_otdr[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBB, S_ANY},
};
operandlist_t operands_otdrx[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xCB, S_ANY},
};
operandlist_t operands_oti2r[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB4, S_ANY},
};
operandlist_t operands_otim[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x83, S_ANY},
};
operandlist_t operands_otimr[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x93, S_ANY},
};
operandlist_t operands_otir[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB3, S_ANY},
};
operandlist_t operands_otirx[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC3, S_ANY},
};
operandlist_t operands_out[] = {
    {OPTYPE_INDIRECT_BC, OPTYPE_R,      false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x41, S_NONE},
    {OPTYPE_INDIRECT_C, OPTYPE_R,       false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x41, S_NONE},
    {OPTYPE_INDIRECT_N, OPTYPE_A,       false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD3, S_NONE},
}; 
operandlist_t operands_out0[] = {
    {OPTYPE_INDIRECT_N, OPTYPE_R,       false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x01, S_NONE},
};
operandlist_t operands_outd[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAB, S_ANY},
};
operandlist_t operands_outd2[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAC, S_ANY},
};
operandlist_t operands_outi[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA3, S_ANY},
};
operandlist_t operands_outi2[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA4, S_ANY},
};
operandlist_t operands_pea[] = {
    {OPTYPE_IXd, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x65, S_ANY},
    {OPTYPE_IYd, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x66, S_ANY},
};
operandlist_t operands_pop[] = {
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0xC1, S_ANY},
    {OPTYPE_AF, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF1, S_ANY},
    {OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE1, S_ANY},
};
operandlist_t operands_push[] = {
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0xC5, S_ANY},
    {OPTYPE_AF, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF5, S_ANY},
    {OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE5, S_ANY},
};
operandlist_t operands_res[] = {
    {OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x86, S_ANY},
    {OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x86, S_ANY},
    {OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_BIT,    TRANSFORM_Z,    0xCB, 0x80, S_NONE},
};
operandlist_t operands_ret[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC9, S_LIL | S_LIS},
    {OPTYPE_CC, OPTYPE_NONE,            false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC0, S_LIL | S_LIS},
};
operandlist_t operands_reti[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x4D, S_LIL | S_LIS},
};
operandlist_t operands_retn[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x45, S_LIL | S_LIS},
};
operandlist_t operands_rl[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x16, S_ANY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x16, S_ANY},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x10, S_NONE},
};
operandlist_t operands_rla[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x17, S_NONE},
};
operandlist_t operands_rlc[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x06, S_ANY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x06, S_ANY},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,     TRANSFORM_NONE,  0xCB, 0x00, S_NONE},
};
operandlist_t operands_rlca[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x07, S_NONE},
};
operandlist_t operands_rld[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6F, S_NONE},
};
operandlist_t operands_rr[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x1E, S_ANY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x1E, S_ANY},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x18, S_NONE},
};
operandlist_t operands_rra[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x1F, S_NONE},
};
operandlist_t operands_rrc[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x0E, S_ANY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x0E, S_ANY},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x08, S_NONE},
};
operandlist_t operands_rrca[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x0F, S_NONE},
};
operandlist_t operands_rrd[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x67, S_NONE},
};
operandlist_t operands_rsmix[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7E, S_NONE},
};
operandlist_t operands_rst[] = {
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_N,      TRANSFORM_NONE, 0x00, 0xC7, S_ANY},
};
operandlist_t operands_sbc[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_ANY},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x9C, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xDE, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x98, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_ANY},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0x9C, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_ANY},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xDE, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0x98, S_NONE},

    {OPTYPE_HL, OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x42, S_ANY},
    {OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x72, S_ANY},
};
operandlist_t operands_scf[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_NONE},
};
operandlist_t operands_set[] = {
    {OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0xC6, S_ANY},
    {OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0xC6, S_ANY},
    {OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_BIT,    TRANSFORM_Z,    0xCB, 0xC0, S_NONE},
};
operandlist_t operands_sla[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x26, S_ANY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x26, S_ANY},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x20, S_NONE},
};
operandlist_t operands_slp[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x76, S_NONE},
};
operandlist_t operands_sra[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x2E, S_ANY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x2E, S_ANY},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x28, S_NONE},
};
operandlist_t operands_srl[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x3E, S_ANY},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x3E, S_ANY},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x38, S_NONE},
};
operandlist_t operands_stmix[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7D, S_NONE},
};
operandlist_t operands_sub[] = {
// optimized set
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,    false, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0x96, S_ANY},
// end optimized set
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x96, S_ANY},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x94, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x96, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD6, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x90, S_NONE},
    // same set, without A register
    {OPTYPE_IR, OPTYPE_NONE,              true, TRANSFORM_IR0,    TRANSFORM_NONE,0x00, 0x94, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,   true, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0x96, S_ANY},
    {OPTYPE_N, OPTYPE_NONE,              false, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0xD6, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,              false, TRANSFORM_Z,      TRANSFORM_NONE,0x00, 0x90, S_NONE},
};
operandlist_t operands_tst[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x34, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x64, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x04, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x34, S_ANY},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x64, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x04, S_NONE},
};
operandlist_t operands_tstio[] = {
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x74, S_NONE},
};
operandlist_t operands_xor[] = {
// optimized set
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xA8, S_NONE},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEE, S_NONE},
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_ANY},
// end optimized set
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_ANY},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xAC, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_ANY},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEE, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xA8, S_NONE},
    // same set, without A register
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xAC, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_ANY},
};

/*
// this table needs to be sorted on name
regcc_t regccs[] = {
    {"a",   R_A,    R_INDEX_A,  false,  0},
    {"af",  R_AF,   R_INDEX_AF, false,  0},
    {"af'", R_AF,   R_INDEX_AF, false,  0},
    {"b",   R_B,    R_INDEX_B,  false,  0},
    {"bc",  R_BC,   R_INDEX_BC, false,  0},
    {"c",   R_C,    R_INDEX_C,  true,   CC_INDEX_C},
    {"d",   R_D,    R_INDEX_D,  false,  0},
    {"de",  R_DE,   R_INDEX_DE, false,  0},
    {"e",   R_E,    R_INDEX_E,  false,  0},
    {"h",   R_H,    R_INDEX_H,  false,  0},
    {"hl",  R_HL,   R_INDEX_HL, false,  0},
    {"i",   R_I,    R_INDEX_I,  false,  0},
    {"ix",  R_IX,   R_INDEX_IX, false,  0},
    {"ixh", R_IXH,  R_INDEX_IX, false,  0},
    {"ixl", R_IXL,  R_INDEX_IX, false,  0},
    {"iy",  R_IY,   R_INDEX_IY, false,  0},
    {"iyh", R_IYH,  R_INDEX_IY, false,  0},
    {"iyl", R_IYL,  R_INDEX_IY, false,  0},
    {"l",   R_L,    R_INDEX_L,  false,  0},
    {"m",   R_NONE, 0,          true,   CC_INDEX_M},
    {"mb",  R_MB,   R_INDEX_MB, false,  0},
    {"nc",  R_NONE, 0,          true,   CC_INDEX_NC},
    {"nz",  R_NONE, 0,          true,   CC_INDEX_NZ},
    {"p",   R_NONE, 0,          true,   CC_INDEX_P},
    {"pe",  R_NONE, 0,          true,   CC_INDEX_PE},
    {"po",  R_NONE, 0,          true,   CC_INDEX_PO},
    {"r",   R_R,    R_INDEX_R,  false,  0},
    {"sp",  R_SP,   R_INDEX_SP, false,  0},
    {"z",   R_NONE, 0,          true,   CC_INDEX_Z}
};
*/
/*
regcc_t *regcc_hashtable[256];

#include "hash.h"
void setup_regcc_hashtable(void) {
    uint16_t n, hash,collisions = 0;
    for(n = 0; n <= 255; n++) {
        regcc_hashtable[n] = NULL;
    }

    for(n = 0; n < (sizeof(regccs) / sizeof(regcc_t)); n++) {
        hash = hash8(regccs[n].name);
        while(regcc_hashtable[hash]) {
            //printf("Collision for %s with existing %s\n", regccs[n].name, regcc_hashtable[hash]->name);
            hash = hash + 1;
            collisions++;
        }
        regcc_hashtable[hash] = &regccs[n];
    }
    // print hashtable
    //printf("%d collisions\n", collisions);
    //for(n = 0; n <= 255; n++) {
    //    if(regcc_hashtable[n]) {
    //        printf("%03d - %s\n", n, regcc_hashtable[n]->name);
    //    }
    //}
}
*/

// this table needs to be sorted on name
instruction_t instructions[] = {
    {"adc",      EZ80, 0, sizeof(operands_adc)/sizeof(operandlist_t), operands_adc},
    {"add",      EZ80, 0, sizeof(operands_add)/sizeof(operandlist_t), operands_add},
    {"align",    ASSEMBLER, ASM_ALIGN, 0, NULL, ASM_ARG_SINGLE},
    {"and",      EZ80, 0, sizeof(operands_and)/sizeof(operandlist_t), operands_and},
    {"ascii",    ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"asciz",    ASSEMBLER, ASM_ASCIZ, 0, NULL, ASM_ARG_LIST},
    {"assume",   ASSEMBLER, ASM_ADL, 0, NULL, ASM_ARG_KEYVAL},
    {"bit",      EZ80, 0, sizeof(operands_bit)/sizeof(operandlist_t), operands_bit},
    {"blkb",     ASSEMBLER, ASM_BLKB, 0, NULL, ASM_ARG_LIST},
    {"blkl",     ASSEMBLER, ASM_BLKL, 0, NULL, ASM_ARG_LIST},
    {"blkp",     ASSEMBLER, ASM_BLKP, 0, NULL, ASM_ARG_LIST},
    {"blkw",     ASSEMBLER, ASM_BLKW, 0, NULL, ASM_ARG_LIST},
    {"byte",     ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"call",     EZ80, 0, sizeof(operands_call)/sizeof(operandlist_t), operands_call},
    {"ccf",      EZ80, 0, sizeof(operands_ccf)/sizeof(operandlist_t), operands_ccf},
    {"cp",       EZ80, 0, sizeof(operands_cp)/sizeof(operandlist_t), operands_cp},
    {"cpd",      EZ80, 0, sizeof(operands_cpd)/sizeof(operandlist_t), operands_cpd},
    {"cpdr",     EZ80, 0, sizeof(operands_cpdr)/sizeof(operandlist_t), operands_cpdr},
    {"cpi",      EZ80, 0, sizeof(operands_cpi)/sizeof(operandlist_t), operands_cpi},
    {"cpir",     EZ80, 0, sizeof(operands_cpir)/sizeof(operandlist_t), operands_cpir},
    {"cpl",      EZ80, 0, sizeof(operands_cpl)/sizeof(operandlist_t), operands_cpl},
    {"daa",      EZ80, 0, sizeof(operands_daa)/sizeof(operandlist_t), operands_daa},
    {"db",       ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"dec",      EZ80, 0, sizeof(operands_dec)/sizeof(operandlist_t), operands_dec},
    {"defb",     ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"defs",     ASSEMBLER, ASM_DS, 0, NULL, ASM_ARG_LIST},
    {"defw",     ASSEMBLER, ASM_DW, 0, NULL, ASM_ARG_LIST},
    {"di",       EZ80, 0, sizeof(operands_di)/sizeof(operandlist_t), operands_di},
    {"djnz",     EZ80, 0, sizeof(operands_djnz)/sizeof(operandlist_t), operands_djnz},
    {"dl",       ASSEMBLER, ASM_DW24, 0, NULL, ASM_ARG_LIST},
    {"ds",       ASSEMBLER, ASM_DS, 0, NULL, ASM_ARG_LIST},
    {"dw",       ASSEMBLER, ASM_DW, 0, NULL, ASM_ARG_LIST},
    {"dw24",     ASSEMBLER, ASM_DW24, 0, NULL, ASM_ARG_LIST},
    {"dw32",     ASSEMBLER, ASM_DW32, 0, NULL, ASM_ARG_LIST},
    {"ei",       EZ80, 0, sizeof(operands_ei)/sizeof(operandlist_t), operands_ei},
    {"else",    ASSEMBLER, ASM_ELSE, 0, NULL, ASM_ARG_NONE},
    {"endif",   ASSEMBLER, ASM_ENDIF, 0, NULL, ASM_ARG_NONE},
    {ENDMACROCMD,ASSEMBLER, ASM_MACRO_END, 0, NULL, ASM_ARG_SINGLE},
    {"equ",      ASSEMBLER, ASM_EQU, 0, NULL, ASM_ARG_SINGLE},
    {"ex",       EZ80, 0, sizeof(operands_ex)/sizeof(operandlist_t), operands_ex},
    {"exx",      EZ80, 0, sizeof(operands_exx)/sizeof(operandlist_t), operands_exx},
    {"fillbyte", ASSEMBLER, ASM_FILLBYTE, 0, NULL, ASM_ARG_SINGLE},
    {"halt",     EZ80, 0, sizeof(operands_halt)/sizeof(operandlist_t), operands_halt},
    {"if",      ASSEMBLER, ASM_IF, 0, NULL, ASM_ARG_SINGLE},
    {"im",       EZ80, 0, sizeof(operands_im)/sizeof(operandlist_t), operands_im},
    {"in",       EZ80, 0, sizeof(operands_in)/sizeof(operandlist_t), operands_in},
    {"in0",      EZ80, 0, sizeof(operands_in0)/sizeof(operandlist_t), operands_in0},
    {"inc",      EZ80, 0, sizeof(operands_inc)/sizeof(operandlist_t), operands_inc},
    {"incbin",   ASSEMBLER, ASM_INCBIN, 0, NULL, ASM_ARG_SINGLE},
    {"include",  ASSEMBLER, ASM_INCLUDE, 0, NULL, ASM_ARG_SINGLE},
    {"ind",      EZ80, 0, sizeof(operands_ind)/sizeof(operandlist_t), operands_ind},
    {"ind2",     EZ80, 0, sizeof(operands_ind2)/sizeof(operandlist_t), operands_ind2},
    {"ind2r",    EZ80, 0, sizeof(operands_ind2r)/sizeof(operandlist_t), operands_ind2r},
    {"indm",     EZ80, 0, sizeof(operands_indm)/sizeof(operandlist_t), operands_indm},
    {"indmr",    EZ80, 0, sizeof(operands_indmr)/sizeof(operandlist_t), operands_indmr},
    {"indr",     EZ80, 0, sizeof(operands_indr)/sizeof(operandlist_t), operands_indr},
    {"indrx",    EZ80, 0, sizeof(operands_indrx)/sizeof(operandlist_t), operands_indrx},
    {"ini",      EZ80, 0, sizeof(operands_ini)/sizeof(operandlist_t), operands_ini},
    {"ini2",     EZ80, 0, sizeof(operands_ini2)/sizeof(operandlist_t), operands_ini2},
    {"ini2r",    EZ80, 0, sizeof(operands_ini2r)/sizeof(operandlist_t), operands_ini2r},
    {"inim",     EZ80, 0, sizeof(operands_inim)/sizeof(operandlist_t), operands_inim},
    {"inimr",    EZ80, 0, sizeof(operands_inimr)/sizeof(operandlist_t), operands_inimr},
    {"inir",     EZ80, 0, sizeof(operands_inir)/sizeof(operandlist_t), operands_inir},
    {"inirx",    EZ80, 0, sizeof(operands_inirx)/sizeof(operandlist_t), operands_inirx},
    {"jp",       EZ80, 0, sizeof(operands_jp)/sizeof(operandlist_t), operands_jp},
    {"jr",       EZ80, 0, sizeof(operands_jr)/sizeof(operandlist_t), operands_jr},
    {"ld",       EZ80, 0, sizeof(operands_ld)/sizeof(operandlist_t), operands_ld},
    {"ldd",      EZ80, 0, sizeof(operands_ldd)/sizeof(operandlist_t), operands_ldd},
    {"lddr",     EZ80, 0, sizeof(operands_lddr)/sizeof(operandlist_t), operands_lddr},
    {"ldi",      EZ80, 0, sizeof(operands_ldi)/sizeof(operandlist_t), operands_ldi},
    {"ldir",     EZ80, 0, sizeof(operands_ldir)/sizeof(operandlist_t), operands_ldir},
    {"lea",      EZ80, 0, sizeof(operands_lea)/sizeof(operandlist_t), operands_lea},
    {"macro",    ASSEMBLER, ASM_MACRO_START, 0, NULL, ASM_ARG_LIST},
    {"mlt",      EZ80, 0, sizeof(operands_mlt)/sizeof(operandlist_t), operands_mlt},
    {"neg",      EZ80, 0, sizeof(operands_neg)/sizeof(operandlist_t), operands_neg},
    {"nop",      EZ80, 0, sizeof(operands_nop)/sizeof(operandlist_t), operands_nop},
    {"or",       EZ80, 0, sizeof(operands_or)/sizeof(operandlist_t), operands_or},
    {"org",     ASSEMBLER, ASM_ORG, 0, NULL, ASM_ARG_SINGLE},
    {"otd2r",    EZ80, 0, sizeof(operands_otd2r)/sizeof(operandlist_t), operands_otd2r},
    {"otdm",     EZ80, 0, sizeof(operands_otdm)/sizeof(operandlist_t), operands_otdm},
    {"otdmr",    EZ80, 0, sizeof(operands_otdmr)/sizeof(operandlist_t), operands_otdmr},
    {"otdr",     EZ80, 0, sizeof(operands_otdr)/sizeof(operandlist_t), operands_otdr},
    {"otdrx",    EZ80, 0, sizeof(operands_otdrx)/sizeof(operandlist_t), operands_otdrx},
    {"oti2r",    EZ80, 0, sizeof(operands_oti2r)/sizeof(operandlist_t), operands_oti2r},
    {"otim",     EZ80, 0, sizeof(operands_otim)/sizeof(operandlist_t), operands_otim},
    {"otimr",    EZ80, 0, sizeof(operands_otimr)/sizeof(operandlist_t), operands_otimr},
    {"otir",     EZ80, 0, sizeof(operands_otir)/sizeof(operandlist_t), operands_otir},
    {"otirx",    EZ80, 0, sizeof(operands_otirx)/sizeof(operandlist_t), operands_otirx},
    {"out",      EZ80, 0, sizeof(operands_out)/sizeof(operandlist_t), operands_out},
    {"out0",     EZ80, 0, sizeof(operands_out0)/sizeof(operandlist_t), operands_out0},
    {"outd",     EZ80, 0, sizeof(operands_outd)/sizeof(operandlist_t), operands_outd},
    {"outd2",    EZ80, 0, sizeof(operands_outd2)/sizeof(operandlist_t), operands_outd2},
    {"outi",     EZ80, 0, sizeof(operands_outi)/sizeof(operandlist_t), operands_outi},
    {"outi2",    EZ80, 0, sizeof(operands_outi2)/sizeof(operandlist_t), operands_outi2},
    {"pea",      EZ80, 0, sizeof(operands_pea)/sizeof(operandlist_t), operands_pea},
    {"pop",      EZ80, 0, sizeof(operands_pop)/sizeof(operandlist_t), operands_pop},
    {"push",     EZ80, 0, sizeof(operands_push)/sizeof(operandlist_t), operands_push},
    {"res",      EZ80, 0, sizeof(operands_res)/sizeof(operandlist_t), operands_res},
    {"ret",      EZ80, 0, sizeof(operands_ret)/sizeof(operandlist_t), operands_ret},
    {"reti",     EZ80, 0, sizeof(operands_reti)/sizeof(operandlist_t), operands_reti},
    {"retn",     EZ80, 0, sizeof(operands_retn)/sizeof(operandlist_t), operands_retn},
    {"rl",       EZ80, 0, sizeof(operands_rl)/sizeof(operandlist_t), operands_rl},
    {"rla",      EZ80, 0, sizeof(operands_rla)/sizeof(operandlist_t), operands_rla},
    {"rlc",      EZ80, 0, sizeof(operands_rlc)/sizeof(operandlist_t), operands_rlc},
    {"rlca",     EZ80, 0, sizeof(operands_rlca)/sizeof(operandlist_t), operands_rlca},
    {"rld",      EZ80, 0, sizeof(operands_rld)/sizeof(operandlist_t), operands_rld},
    {"rr",       EZ80, 0, sizeof(operands_rr)/sizeof(operandlist_t), operands_rr},
    {"rra",      EZ80, 0, sizeof(operands_rra)/sizeof(operandlist_t), operands_rra},
    {"rrc",      EZ80, 0, sizeof(operands_rrc)/sizeof(operandlist_t), operands_rrc},
    {"rrca",     EZ80, 0, sizeof(operands_rrca)/sizeof(operandlist_t), operands_rrca},
    {"rrd",      EZ80, 0, sizeof(operands_rrd)/sizeof(operandlist_t), operands_rrd},
    {"rsmix",    EZ80, 0, sizeof(operands_rsmix)/sizeof(operandlist_t), operands_rsmix},
    {"rst",      EZ80, 0, sizeof(operands_rst)/sizeof(operandlist_t), operands_rst},
    {"sbc",      EZ80, 0, sizeof(operands_sbc)/sizeof(operandlist_t), operands_sbc},
    {"scf",      EZ80, 0, sizeof(operands_scf)/sizeof(operandlist_t), operands_scf},
    {"set",      EZ80, 0, sizeof(operands_set)/sizeof(operandlist_t), operands_set},
    {"sla",      EZ80, 0, sizeof(operands_sla)/sizeof(operandlist_t), operands_sla},
    {"slp",      EZ80, 0, sizeof(operands_slp)/sizeof(operandlist_t), operands_slp},
    {"sra",      EZ80, 0, sizeof(operands_sra)/sizeof(operandlist_t), operands_sra},
    {"srl",      EZ80, 0, sizeof(operands_srl)/sizeof(operandlist_t), operands_srl},
    {"stmix",    EZ80, 0, sizeof(operands_stmix)/sizeof(operandlist_t), operands_stmix},
    {"sub",      EZ80, 0, sizeof(operands_sub)/sizeof(operandlist_t), operands_sub},
    {"tst",      EZ80, 0, sizeof(operands_tst)/sizeof(operandlist_t), operands_tst},
    {"tstio",    EZ80, 0, sizeof(operands_tstio)/sizeof(operandlist_t), operands_tstio},
    {"xor",      EZ80, 0, sizeof(operands_xor)/sizeof(operandlist_t), operands_xor}
};

/*
regcc_t * regcc_table_lookup(char *key) {
	regcc_t *base = regccs;
	int lim, cmp;
	regcc_t *p;

	for (lim = sizeof(regccs)/sizeof(regcc_t); lim != 0; lim >>= 1) {
		p = base + (lim >> 1);
		cmp = strcasecmp(key,p->name);
		if (cmp == 0)
			return p;
		if (cmp > 0) {
			base = p + 1;
			lim--;
		}
	}
	return (NULL);
}
*/

/*
regcc_t * regcc_table_lookup(char *name) {
    uint8_t index,i;
    uint8_t try;
    index = hash8(name);
    for(i = 0; i < 255; i++){
        try = (index + i) & 0xFF;
        if(regcc_hashtable[try] == NULL){
            return NULL;
        }
        if(regcc_hashtable[try] != NULL &&
            strcasecmp(regcc_hashtable[try]->name,name) == 0){
            return regcc_hashtable[try];
        }
    }
    return NULL;
}
*/

// Binary search of instruction_t table
// Requires a pre-sorted table
instruction_t * instruction_table_lookup(char *key){
	instruction_t *base = instructions;
	int lim, cmp;
	instruction_t *p;

	for (lim = sizeof(instructions)/sizeof(instruction_t); lim != 0; lim >>= 1) {
		p = base + (lim >> 1);
		cmp = strcasecmp(key,p->name);
		if (cmp == 0)
			return p;
		if (cmp > 0) {
			base = p + 1;
			lim--;
		}
	}
	return (NULL);
}

