#include "assemble.h"
#include <time.h>

// Local file buffer
char _buffer[FILE_BUFFERSIZE];
char _incbuffer[FILESTACK_MAXFILES][FILE_BUFFERSIZE];
char _macrobuffer[MACRO_BUFFERSIZE];
char _macro_OP_buffer[MACROARGLENGTH + 1]; // replacement buffer for operands during macro expansion
char _macro_ASM_buffer[MACROARGLENGTH + 1];// replacement buffer for values during ASSEMBLER macro expansion

uint8_t pass2matchlog[PASS2LOGSIZE];
uint8_t *pass2matchlogptr;
uint24_t passmatchcounter;

struct incbinitem incbins[INCBINBUFFERMAX];
uint8_t incbincounter;

// Parse a command-token string to currentline.mnemonic & currentline.suffix
void parse_command(char *src) {
    currentline.mnemonic = src;

    while(*src && (*src != '.')) src++;
    if(*src) {
        // suffix start found
        *src = 0; // terminate mnemonic
        currentline.suffixpresent = true;
        currentline.suffix = src + 1;
        return;
    }
    // no suffix found
    currentline.suffixpresent = false;
    currentline.suffix = NULL;
    return;
}

// parses the given string to the operand, or throws errors along the way
// will destruct parts of the original string during the process
void parse_operand(char *string, uint8_t len, operand_t *operand) {
    char *ptr = string;

    operand->addressmode = NOREQ;
    operand->reg = R_NONE;

    // direct or indirect
    if(*ptr == '(') {
        operand->indirect = true;
        operand->addressmode |= INDIRECT;
        // find closing bracket or error out
        if(string[len-1] == ')') string[len-1] = 0; // terminate on closing bracket
        else error(message[ERROR_CLOSINGBRACKET]);
        ptr = &string[1];
        while(isspace(*ptr)) ptr++; // eat spaces
    }
    else {
        operand->indirect = false;
        // should not find a closing bracket
        if(string[len-1] == ')') error(message[ERROR_OPENINGBRACKET]);
    }
    
    switch(*ptr++) {
        case 0: // empty operand
            break;
        case 'a':
        case 'A':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_A;
                    operand->reg_index = R_INDEX_A;
                    return;
                case 'f':
                case 'F':
                    switch(*ptr++) {
                        case 0:
                        case '\'':
                            operand->reg = R_AF;
                            operand->reg_index = R_INDEX_AF;
                            return;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'b':
        case 'B':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_B;
                    operand->reg_index = R_INDEX_B;
                    return;
                case 'c':
                case 'C':
                    if((*ptr == 0) || isspace(*ptr)) {
                        operand->reg = R_BC;
                        operand->reg_index = R_INDEX_BC;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'c':
        case 'C':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_C;
                    operand->reg_index = R_INDEX_C;
                    operand->cc = true;
                    operand->cc_index = CC_INDEX_C;
                    return;
                default:
                    break;
            }
            break;
        case 'd':
        case 'D':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_D;
                    operand->reg_index = R_INDEX_D;
                    return;
                case 'e':
                case 'E':
                    if((*ptr == 0) || isspace(*ptr)) {
                        operand->reg = R_DE;
                        operand->reg_index = R_INDEX_DE;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'e':
        case 'E':
            if(*ptr++ == 0) {
                operand->reg = R_E;
                operand->reg_index = R_INDEX_E;
                return;
            }
            break;
        case 'h':
        case 'H':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_H;
                    operand->reg_index = R_INDEX_H;
                    return;
                case 'l':
                case 'L':
                    if((*ptr == 0) || isspace(*ptr)) {
                        operand->reg = R_HL;
                        operand->reg_index = R_INDEX_HL;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'i':
        case 'I':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_I;
                    operand->reg_index = R_INDEX_I;
                    return;
                case 'x':
                case 'X':
                    while(isspace(*ptr)) ptr++; // eat spaces
                    switch(*ptr++) {
                        case 0:
                            operand->reg = R_IX;
                            operand->reg_index = R_INDEX_IX;
                            return;
                        case 'h':
                        case 'H':
                            if(*ptr == 0) {
                                operand->reg = R_IXH;
                                return;
                            }
                            break;
                        case 'l':
                        case 'L':
                            if(*ptr == 0) {
                                operand->reg = R_IXL;
                                return;
                            }
                            break;
                        case '+':
                        case '-':
                            operand->reg = R_IX;
                            operand->displacement_provided = true;
                            //operand->addressmode |= DISPLACEMENT;
                            if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) getValue(ptr, false);
                            else operand->displacement = (int16_t) getValue(ptr, false);
                            return;
                            break;
                        default:
                            break;
                    }
                    break;
                case 'y':
                case 'Y':
                    while(isspace(*ptr)) ptr++; // eat spaces
                    switch(*ptr++) {
                        case 0:
                            operand->reg = R_IY;
                            operand->reg_index = R_INDEX_IY;
                            return;
                        case 'h':
                        case 'H':
                            if(*ptr == 0) {
                                operand->reg = R_IYH;
                                return;
                            }
                            break;
                        case 'l':
                        case 'L':
                            if(*ptr == 0) {
                                operand->reg = R_IYL;
                                return;
                            }
                            break;
                        case '+':
                        case '-':
                            operand->reg = R_IY;
                            operand->displacement_provided = true;
                            //operand->addressmode |= DISPLACEMENT;
                            if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) getValue(ptr, false);
                            else operand->displacement = (int16_t) getValue(ptr, false);
                            return;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'l':
        case 'L':
            if(*ptr == 0) {
                operand->reg = R_L;
                operand->reg_index = R_INDEX_L;
                return;
            }
            break;
        case 'm':
        case 'M':
            if((tolower(*ptr) == 'b') && ptr[1] == 0) {
                operand->reg = R_MB;
                operand->reg_index = R_INDEX_MB;
                return;
            }
            if(*ptr == 0) {
                operand->cc = true;
                operand->addressmode |= CC;
                operand->cc_index = CC_INDEX_M;
                return;
            }
            break;
        case 'n':
        case 'N':
            switch(*ptr++) {
                case 'c':   // NC
                case 'C':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->addressmode |= CC;
                        operand->cc_index = CC_INDEX_NC;
                        operand->addressmode |= CCA;
                        return;
                    }
                    break;
                case 'z':   // NZ
                case 'Z':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->addressmode |= CC;
                        operand->cc_index = CC_INDEX_NZ;
                        operand->addressmode |= CCA;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'p':
        case 'P':
            switch(*ptr++) {
                case 0:
                    operand->cc = true;
                    operand->addressmode |= CC;
                    operand->cc_index = CC_INDEX_P;
                    return;
                case 'e':
                case 'E':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->addressmode |= CC;
                        operand->cc_index = CC_INDEX_PE;
                        return;
                    }
                    break;
                case 'o':
                case 'O':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->addressmode |= CC;
                        operand->cc_index = CC_INDEX_PO;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'r':
        case 'R':
            if(*ptr == 0) {
                operand->reg = R_R;
                operand->reg_index = R_INDEX_R;
                return;
            }
            break;
        case 's':
        case 'S':
            if((tolower(*ptr) == 'p') && ptr[1] == 0) {
                operand->reg = R_SP;
                operand->reg_index = R_INDEX_SP;
                return;
            }
            break;
        case 'z':
        case 'Z':
            if(*ptr == 0) {
                operand->cc = true;
                operand->addressmode |= CC;
                operand->cc_index = CC_INDEX_Z;
                operand->addressmode |= CCA;
                return;
            }
            break;
        default:
            break;
    }
    
    if(*string) {
        if(operand->indirect) string++;
        operand->immediate = getValue(string, false);
        operand->immediate_provided = true;
        operand->addressmode |= IMM;
    }
}

// FSM to parse each line into separate components, store in gbl currentline variable
void parseLine(char *src) {
    uint8_t oplength = 0;
    uint8_t x;
    bool done;
    bool asmcmd = false;
    uint8_t state;
    uint8_t argcount = 0;
    streamtoken_t streamtoken;

    // default current line items
    memset(&currentline, 0, sizeof(currentline));
    memset(&operand1, 0, sizeof(operand_t));
    memset(&operand2, 0, sizeof(operand_t));

    state = PS_START;
    done = false;
    while(!done) {
        switch(state) {
            case PS_START:
                if((isspace(*src) == 0) && (*src) != '.') {
                    // LABEL or COMMENT
                    getLabelToken(&streamtoken, src);
                    switch(streamtoken.terminator) {
                        case ':':
                            state = PS_LABEL;
                            break;
                        case ';':
                            if(strlen(streamtoken.start) == 0) {
                                currentline.next = streamtoken.next;
                                state = PS_COMMENT;
                                break;
                            }
                        default: // intentional fall-through
                            error(message[ERROR_INVALIDLABEL]);
                            state = PS_ERROR;                        
                            break;
                    }
                    break;
                }
                // COMMAND or COMMENT
                x = getMnemonicToken(&streamtoken, src);
                if(x) {
                    state = PS_COMMAND;
                    break;
                }
                else {
                    if(streamtoken.terminator == 0) {
                        state = PS_DONE;
                        break;
                    }
                    if(streamtoken.terminator == ';') {
                        state = PS_COMMENT;
                        break;
                    }
                }
                break;
            case PS_LABEL:
                currentline.label = streamtoken.start;
                advanceAnonymousLabel();
                x = getMnemonicToken(&streamtoken, streamtoken.next);
                if(x) state = PS_COMMAND;
                else {
                    if(streamtoken.terminator == 0) {
                        state = PS_DONE;
                        break;
                    }
                    if(streamtoken.terminator == ';') {
                        state = PS_COMMENT;
                        currentline.next = streamtoken.next;
                        break;
                    }
                }
                break;
            case PS_COMMAND:
                if(streamtoken.start[0] == '.') {
                    // should be an assembler command
                    asmcmd = true;
                    currentline.mnemonic = streamtoken.start;
                }
                else parse_command(streamtoken.start); // ez80 split suffix and set mnemonic for search

                currentline.current_instruction = instruction_lookup(currentline.mnemonic);
                if(currentline.current_instruction == NULL) {
                    if(!asmcmd) {
                        error(message[ERROR_INVALIDMNEMONIC]);
                        state = PS_ERROR;
                        break;
                    }
                    // Check for assembler command
                    currentline.mnemonic = streamtoken.start + 1;
                    currentline.current_instruction = instruction_lookup(currentline.mnemonic);
                    if((currentline.current_instruction == NULL) ||
                       (currentline.current_instruction->type != ASSEMBLER)) {
                        error(message[ERROR_INVALIDMNEMONIC]);
                        state = PS_ERROR;
                        break;
                    }
                    // Valid assembler command found (with a .)
                }
                switch(currentline.current_instruction->type) {
                    case EZ80:
                        switch(streamtoken.terminator) {
                            case ';':
                                state = PS_COMMENT;
                                currentline.next = streamtoken.next;
                                break;
                            case 0:
                                currentline.next = NULL;
                                state = PS_DONE;
                                break;
                            default:
                                if(streamtoken.next) {
                                    oplength = getOperandToken(&streamtoken, streamtoken.next);
                                    if(oplength) {
                                        state = PS_OP1;
                                        break;
                                    }
                                }
                                state = PS_DONE; // ignore any comments
                                break;
                        }
                        break;
                    case ASSEMBLER:
                        currentline.next = streamtoken.next;
                        state = PS_DONE;
                        break;
                    case MACRO:
                        currentline.current_macro = currentline.current_instruction->macro;
                        currentline.current_instruction = NULL;
                        currentline.next = streamtoken.next;
                        state = PS_DONE;
                        break;
                }
                break;
            case PS_OP1:
                argcount++;                
                if(currentExpandedMacro) {
                    oplength = macroExpandArg(_macro_OP_buffer, streamtoken.start, currentExpandedMacro);
                    if(oplength) {
                        streamtoken.start = _macro_OP_buffer;
                    }
                }
                if(argcount == 1) {
                    parse_operand(streamtoken.start, oplength, &operand1);
                }
                else {
                    parse_operand(streamtoken.start, oplength, &operand2);
                }
                switch(streamtoken.terminator) {
                    case ';':
                        currentline.next = streamtoken.next;
                        state = PS_COMMENT;
                        break;
                    case 0:
                        currentline.next = NULL;
                        state = PS_DONE;
                        break;
                    case ',':
                        if(argcount == 2) {
                            error(message[ERROR_TOOMANYARGUMENTS]);
                            state = PS_ERROR;
                            break;
                        }
                        oplength = getOperandToken(&streamtoken, streamtoken.next);
                        if(oplength == 0) {
                            error(message[ERROR_MISSINGOPERAND]);
                            state = PS_ERROR;
                            break;
                        }
                        break;
                }
                break;
            case PS_COMMENT:
                currentline.comment = currentline.next;
                state = PS_DONE;
                break;
            case PS_ERROR:
                currentline.next = NULL;
                state = PS_DONE;
                break;
            case PS_DONE:
                done = true;
                break;
        }
    }
}

// Parse an immediate value from currentline.next
// services several assembler directives
void parse_asm_single_immediate(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getOperandToken(&token, currentline.next)) {
            operand1.immediate = getValue(token.start, true);
            operand1.immediate_provided = true;
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
        }
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

// Emits list data for the DB/DW/DW24/DW32 etc directives
void handle_asm_data(uint8_t wordtype) {
    int32_t value;
    streamtoken_t token;
    bool expectarg = true;

    definelabel(address);

    while(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {

            if(currentExpandedMacro) {
                if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                    token.start = _macro_ASM_buffer;
                }
            }

            if((token.start[0] == '\"') && (wordtype != ASM_DB)) {
                error(message[ERROR_STRING_NOTALLOWED]);
                return;
            }

            switch(wordtype) {
                case ASM_DB:
                    switch(token.start[0]) {
                        case '\"':
                            emit_quotedstring(token.start);
                            break;
                        default:
                            value = getValue(token.start, false); // not needed in pass 1
                            if(pass == 2) validateRange8bit(value);
                            emit_8bit(value);
                            break;
                    }
                    break;
                case ASM_DW:
                    value = getValue(token.start, false);
                    if(pass == 2) validateRange16bit(value);
                    emit_16bit(value);
                    break;
                case ASM_DW24:
                    value = getValue(token.start, false);
                    if(pass == 2) validateRange24bit(value);
                    emit_24bit(value);
                    break;
                case ASM_DW32:
                    value = getValue(token.start, false);
                    emit_32bit(value);
                    break;
                default:
                    error(message[ERROR_INTERNAL]);
                    break;
            }
            expectarg = false;
        }
        if(token.terminator == ',') {
            currentline.next = token.next;
            expectarg = true;
        }
        else {
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
            currentline.next = NULL;
        }
    }
    if(expectarg) error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_equ(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
            if(currentline.label) definelabel(getValue(token.start, true)); // needs to be defined in pass 1
            else error(message[ERROR_MISSINGLABEL]);
        }
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_adl(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getDefineValueToken(&token, currentline.next) == 0) {
            error(message[ERROR_MISSINGOPERAND]);
            return;
        }
        if(currentExpandedMacro) {
            if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                token.start = _macro_ASM_buffer;
            }
        }

        if(fast_strcasecmp(token.start, "adl")) {
            error(message[ERROR_INVALIDOPERAND]);
            return;
        }
        if(token.terminator == '=') {
            if(getDefineValueToken(&token, token.next)) {
                operand2.immediate = getValue(token.start, true); // needs to be defined in pass 1
                operand2.immediate_provided = true;
            }
            else error(message[ERROR_MISSINGOPERAND]);
        }        
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);


    if((operand2.immediate != 0) && (operand2.immediate != 1)) {
        error(message[ERROR_INVALID_ADLMODE]);
    }

    adlmode = operand2.immediate;
}

void handle_asm_org(void) {
    uint24_t newaddress;
    
    parse_asm_single_immediate(); // get address from next token
    // address needs to be given in pass 1
    newaddress = operand1.immediate;
    if((adlmode == 0) && (newaddress > 0xffff)) error(message[ERROR_ADDRESSRANGE]); 
    definelabel(address);

    address = newaddress;
}

void handle_asm_include(void) {
    streamtoken_t token;
    filestackitem fsi;
    uint8_t inclevel;
    
    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND]);
        return;
    }
    inclevel = filestackCount();
    if(inclevel > FILESTACK_MAXFILES) {
        error(message[ERROR_MAXINCLUDEFILES]);
        return;
    }
    getDefineValueToken(&token, currentline.next);
    if(token.start[0] != '\"') {
        error(message[ERROR_STRINGFORMAT]);
        return;
    }

    token.start[strlen(token.start)-1] = 0;
    io_getCurrent(&fsi);
    filestackPush(&fsi);
    strcpy(fsi.filename, token.start+1);
    // set new file
    io_getFileDefaults(&fsi);
    fsi.fp = fopen(token.start+1, "rb");
    strncpy(fsi.filename, token.start+1, sizeof(fsi.filename));
    fsi.filename[sizeof(fsi.filename)-1] = '\0';
    fsi.bufferstart = &_incbuffer[inclevel][0];
    fsi.filebuffer = fsi.bufferstart;
    io_setCurrent(&fsi);
    
    if(filehandle[FILE_CURRENT] == 0) {
        error(message[ERROR_INCLUDEFILE]);
        filestackPop(&fsi);
    }
    lineNumberNeedsReset = true;
    if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
}

void handle_asm_incbin(void) {
    streamtoken_t token;
    FILE* fh;
    uint24_t n, filesize, size = 0;

    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND]);
        return;
    }

    getDefineValueToken(&token, currentline.next);

    if(currentExpandedMacro) {
        if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
            token.start = _macro_ASM_buffer;
        }
    }

    if(token.start[0] != '\"') {
        error(message[ERROR_STRINGFORMAT]);
        return;
    }

    token.start[strlen(token.start)-1] = 0;

    if(pass == 1) {
        fh = fopen(token.start+1, "rb");
        if(!fh) {
            error(message[ERROR_INCLUDEFILE]);            
            return;
        }
        #ifdef CEDEV
            // Use optimal assembly routine in moscalls.asm
            filesize = getfilesize(fh->fhandle);
            address += filesize;
        #else
            filesize = 0;
            while(1) {
                // Other non-agon compilers
                size = fread(_buffer, 1, FILE_BUFFERSIZE, fh);
                filesize += size;
                address += size;
                if(size < FILE_BUFFERSIZE) break;
            }
            fseek(fh, 0, SEEK_SET);
        #endif
        if(incbincounter < INCBINBUFFERMAX) {
            incbins[incbincounter].size = filesize;
            incbins[incbincounter].buffer = (char *)malloc(filesize);
            if(incbins[incbincounter].buffer == NULL) {
                error(message[ERROR_MEMORY]);
                return;
            }
            size = fread(incbins[incbincounter].buffer, 1, filesize, fh);
            if(size != incbins[incbincounter].size) {
                error(message[ERROR_READINGBINFILE]);
                return;
            }
            incbincounter++;
        }
        fclose(fh);
    }

    if(pass == 2) {
        if(incbincounter < INCBINBUFFERMAX) { // read from buffer
            size = incbins[incbincounter].size;
            if(list_enabled || consolelist_enabled) { // Output needs to pass to the listing through emit_8bit, performance-hit
                for(n = 0; n < size; n++) emit_8bit(incbins[incbincounter].buffer[n]);
            }
            else {
                io_write(FILE_OUTPUT, incbins[incbincounter].buffer, size);
                address += size;
            }
            incbincounter++;        
        }
        else { // read from file
            fh = fopen(token.start+1, "rb");
            if(!fh) {
                error(message[ERROR_INCLUDEFILE]);            
                return;
            }
            while(1) {
                size = fread(_buffer, 1, FILE_BUFFERSIZE, fh);
                if(list_enabled || consolelist_enabled) { // Output needs to pass to the listing through emit_8bit, performance-hit
                    for(n = 0; n < size; n++) emit_8bit(_buffer[n]);
                }
                else {
                    io_write(FILE_OUTPUT, _buffer, size);
                    address += size;
                }
                if(size < FILE_BUFFERSIZE) break;
            }
            fclose(fh);
        }
        binfilecount++;
    }
    if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
}

void handle_asm_blk(uint8_t width) {
    uint24_t num;
    int32_t val = 0;
    streamtoken_t token;

    definelabel(address);

    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND]);
        return;
    }

    if(getDefineValueToken(&token, currentline.next) == 0) {
        error(message[ERROR_MISSINGOPERAND]); // we need at least one value
        return;
    }

    if(currentExpandedMacro) {
        if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
            token.start = _macro_ASM_buffer;
        }
    }

    num = getValue(token.start, true); // <= needs a number of items during pass 1, otherwise addresses will be off later on

    if(token.terminator == ',') {
        if(getDefineValueToken(&token, token.next) == 0) {
            error(message[ERROR_MISSINGOPERAND]);
            return;
        }

        if(currentExpandedMacro) {
            if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                token.start = _macro_ASM_buffer;
            }
        }
        val = getValue(token.start, false); // value not required in pass 1
    }
    else { // no value given
        if((token.terminator != 0)  && (token.terminator != ';'))
            error(message[ERROR_LISTFORMAT]);
        val = fillbyte;
    }
    while(num--) {
        switch(width) {
            case 1:
                if(pass == 2) validateRange8bit(val);
                emit_8bit(val);
                break;
            case 2:
                if(pass == 2) validateRange16bit(val);
                emit_16bit(val);
                break;
            case 3:
                if(pass == 2) validateRange24bit(val);
                emit_24bit(val);
                break;
            case 4:
                emit_32bit(val);
                break;
        }
    }
}

void handle_asm_align(void) {
uint24_t alignment;
uint24_t base;
uint24_t delta;

    parse_asm_single_immediate();
    if(operand1.immediate <= 0) {
        error(message[ERROR_INVALIDNUMBER]);
        return;
    }

    if((operand1.immediate & (operand1.immediate - 1)) != 0) {
        error(message[ERROR_POWER2]); 
        return;
    }
    
    alignment = operand1.immediate;
    base = (~(operand1.immediate - 1) & address);

    if(address & (operand1.immediate -1)) base += alignment;
    delta = base - address;
    while(delta--) emit_8bit(fillbyte);

    address = base;
    definelabel(address); // set address to current line
}

// strncasecmp is boken on CEDev
/*
int agon_strncasecmp(char *s1, char *s2, int n) {
  if (n == 0)
    return 0;

  while (n-- != 0 && tolower(*s1) == tolower(*s2))
    {
      if (n == 0 || *s1 == '\0' || *s2 == '\0')
    break;
      s1++;
      s2++;
    }

  return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}
*/
void handle_asm_definemacro(void) {
    streamtoken_t token;
    uint8_t argcount = 0;
    char arglist[MACROMAXARGS][MACROARGLENGTH + 1];
    char macroline[LINEMAX];
    char *strend;
    definelabel(address);
    bool foundend = false;
    macro_t *macro;

    _macrobuffer[0] = 0; // empty string
    strend = _macrobuffer;

    if(pass == 2 && (consolelist_enabled || list_enabled)) listEndLine(); // print out first line of macro definition

    while(io_getline(FILE_CURRENT, macroline)) {
        linenumber++;
        char *src = macroline;

        if(pass == 2 && (consolelist_enabled || list_enabled)) {
            listStartLine(macroline);
            listEndLine();
        }
        if((!isspace(src[0])) && (src[0] != '@') && (src[0] != ';')) {
            error(message[ERROR_MACRO_NOGLOBALLABELS]);
            break;
        }
        // skip leading space
        while(*src && (isspace(*src))) src++;
        if(fast_strncasecmp(src, "macro", 5) == 0) {
            error(message[ERROR_MACROINMACRO]);
            break;
        }
        if(fast_strncasecmp(src, "endmacro", 8) == 0) {
            if(isspace(src[8]) || (src[8] == 0) || (src[8] == ';')) {
                foundend = true;
                break;
            }
        }
        // concatenate to buffer end
        if(pass == 1) {
            char *tmp = macroline;
            while(*tmp) *strend++ = *tmp++;
            *strend = 0;
        }
    }
    if(!foundend) {
        error(message[ERROR_MACROUNFINISHED]);
        return;
    }

    // Only define macros in pass 1
    // parse arguments into array
    if(pass == 1) {
        if(!currentline.next) {
            error(message[ERROR_MACRONAME]);
            return;
        }
        if(getMnemonicToken(&token, currentline.next) == 0) { // terminate on space
            error(message[ERROR_MACRONAME]);
            return;
        }
        currentline.mnemonic = token.start;

        currentline.next = token.next;
        if((token.terminator == ' ') || (token.terminator == '\t')) {
            while(currentline.next) {
                if(argcount == MACROMAXARGS) error(message[ERROR_MACROARGCOUNT]);
                if(getDefineValueToken(&token, currentline.next)) {
                    strcpy(arglist[argcount], token.start);
                    argcount++;
                }
                if(token.terminator == ',') currentline.next = token.next;
                else {
                    if((token.terminator != 0) &&(token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
                    currentline.next = NULL; 
                }
            }
        }
        // record the macro to memory
        macro = defineMacro(currentline.mnemonic, argcount, (char *)arglist);
        if(!macro) {
            error(message[ERROR_MACROMEMORYALLOCATION]);
            return;
        }
        setMacroBody(macro, _macrobuffer);
    }
}

void handle_asm_if(void) {
    streamtoken_t token;
    int24_t value;
    
    // No nested conditionals.
    if(inConditionalSection != 0) {
        error(message[ERROR_NESTEDCONDITIONALS]);
        return;
    }

    if(currentline.next) {
        if(getMnemonicToken(&token, currentline.next) == 0) { // terminate on space
            error(message[ERROR_CONDITIONALEXPRESSION]);
            return;
        }
        value = getValue(token.start, true);

        inConditionalSection = value ? 2 : 1;
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_else(void) {
    // No nested conditionals.
    if(inConditionalSection == 0) {
        error(message[ERROR_MISSINGIFCONDITION]);
        return;
    }
    inConditionalSection = inConditionalSection == 1 ? 2 : 1;
}

void handle_asm_endif(void) {
    if(inConditionalSection == 0) {
        error(message[ERROR_MISSINGIFCONDITION]);
        return;
    }
    inConditionalSection = 0;
}

void handle_asm_fillbyte(void) {
    parse_asm_single_immediate(); // get fillbyte from next token
    if((operand1.immediate < 0) || (operand1.immediate > 255)) error(message[ERROR_8BITRANGE]);
    fillbyte = operand1.immediate;
}

void handle_assembler_command(void) {
    switch(currentline.current_instruction->asmtype) {
        case(ASM_ADL):
            handle_asm_adl();
            break;
        case(ASM_ORG):
            handle_asm_org();
            break;
        case(ASM_DB):
            handle_asm_data(ASM_DB);
            break;
        case(ASM_DS):
            handle_asm_blk(1);
            break;
        case(ASM_DW):
            handle_asm_data(ASM_DW);
            break;
        case(ASM_DW24):
            handle_asm_data(ASM_DW24);
            break;
        case(ASM_DW32):
            handle_asm_data(ASM_DW32);
            break;
        case(ASM_ASCIZ):
            handle_asm_data(ASM_DB);
            emit_8bit(0);
            break;
        case(ASM_EQU):
            handle_asm_equ();
            break;
        case(ASM_INCLUDE):
            handle_asm_include();
            break;
        case(ASM_BLKB):
            handle_asm_blk(1);
            break;
        case(ASM_BLKW):
            handle_asm_blk(2);
            break;
        case(ASM_BLKP):
            handle_asm_blk(3);
            break;
        case(ASM_BLKL):
            handle_asm_blk(4);
            break;
        case(ASM_ALIGN):
            handle_asm_align();
            break;
        case(ASM_MACRO_START):
            handle_asm_definemacro();
            break;
        case(ASM_INCBIN):
            handle_asm_incbin();
            break;
        case(ASM_FILLBYTE):
            handle_asm_fillbyte();
            break;
        case(ASM_IF):
            handle_asm_if();
            break;
        case(ASM_ELSE):
            handle_asm_else();
            break;
        case(ASM_ENDIF):
            handle_asm_endif();
            break;
    }
    return;
}

// Process the instructions found at each line, after parsing them
void processInstructions(void){
    operandlist_t *list;
    uint8_t listitem;
    bool match;
    bool condmatch;
    bool regamatch, regbmatch;

    if(currentline.mnemonic == NULL) definelabel(address);
    
    if(currentline.current_instruction) {
        if(currentline.current_instruction->type == EZ80) {
            if(inConditionalSection != 1) {
                // process this mnemonic by applying the instruction list as a filter to the operand-set
                list = currentline.current_instruction->list;
                if(pass == 1) {
                    match = false;
                    for(listitem = 0; listitem < currentline.current_instruction->listnumber; listitem++) {
                        regamatch = (list->regsetA & operand1.reg) || !(list->regsetA | operand1.reg);
                        regbmatch = (list->regsetB & operand2.reg) || !(list->regsetB | operand2.reg);

                        condmatch = ((list->conditionsA & MODECHECK) == operand1.addressmode) && ((list->conditionsB & MODECHECK) == operand2.addressmode);
                        if(list->flags & F_CCOK) {
                            condmatch |= operand1.cc;
                            regamatch = true;
                        }
                        /*
                        if(debug) {
                            printf("Index list [[%d]]\r\n", listitem);
                            printf("regamatch: %d\r\n", regamatch);
                            printf("regbmatch: %d\r\n", regbmatch);
                            printf("condmatch: %d\r\n", condmatch);
                            printf("regsetA: <0x%03X> - regsetB <0x%03X>\r\n", list->regsetA, list->regsetB);
                            printf("    opA: <0x%03X> -     opB <0x%03X>\r\n", operand1.reg, operand2.reg);
                            printf("  condA: <0x%0X>  -   condB <0x%0X>\r\n", list->conditionsA, list->conditionsB);
                            printf("    opA: <0x%0X>  -     opB <0x%0X>\r\n", operand1.addressmode, operand2.addressmode);
                            printf("--------------------------------------\r\n");
                        }
                        */
                        if(regamatch && regbmatch && condmatch) {
                            match = true;
                            emit_instruction(list);
                            *pass2matchlogptr++ = listitem; // record log for pass 2
                            passmatchcounter++;
                            if(passmatchcounter == PASS2LOGSIZE) {
                                error(message[ERROR_MAXINSTRUCTIONS]);
                            }
                            break;
                        }
                        list++;
                    }
                    if(!match) error(message[ERROR_OPERANDSNOTMATCHING]);
                    return;
                }
                list += *pass2matchlogptr++;
                emit_instruction(list);
            }
        }
        else handle_assembler_command();
    }
    return;
}

void processMacro(void) {
    streamtoken_t token;
    uint8_t argcount = 0;
    macro_t *exp = currentline.current_macro;
    char macroline[LINEMAX];

    // set for additional line-based parsing/processing of the macro
    currentExpandedMacro = currentline.current_macro;

    // get arguments
    while(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {
            argcount++;
            if(argcount > exp->argcount) {
                error(message[ERROR_MACROARGCOUNT]);
                return;
            }
            strcpy(exp->substitutions[argcount-1], token.start);
        }
        if(token.terminator == ',') currentline.next = token.next;
        else {
            if((token.terminator != 0) &&(token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
            currentline.next = NULL; 
        }
    }
    if(argcount != exp->argcount) error(message[ERROR_MACROINCORRECTARG]);

    // open macro storage
    resetnextline(exp->body);

    // process body
    while(getnextline(macroline)) {
        if(pass == 2 && (consolelist_enabled || list_enabled)) listStartLine(macroline);
        parseLine(macroline);
        processInstructions();
        if(pass == 2 && (consolelist_enabled || list_enabled)) listEndLine();
    }

    // end processing
    currentExpandedMacro = NULL;
}

// Initialize pass 1 / pass2 states for the assembler
void passInitialize(uint8_t passnumber) {
    pass = passnumber;
    linenumber = 0;
    address = start_address;
    currentExpandedMacro = NULL;
    inConditionalSection = 0;
    io_setpass(passnumber);
    filestackInit();
    initAnonymousLabelTable();
    io_resetCurrentInput();
    pass2matchlogptr = pass2matchlog;
    if(pass == 1) passmatchcounter = 0;
    incbincounter = 0;
}

// Assembler directives may demand a late reset of the linenumber, after the listing has been done
void processDelayedLineNumberReset(void) {
    if(lineNumberNeedsReset) {
        lineNumberNeedsReset = false;
        linenumber = 0;
    }
}

bool assemble(void){
    char line[LINEMAX]; // Temp line buffer, will be deconstructed during streamtoken_t parsing
    char errorline[LINEMAX]; // Full integrity copy of each line
    //char macroline[LINEMAX]; // Temp line buffer for macro expansion
    filestackitem fsitem;
    bool incfileState;
    global_errors = 0;
    
    total = 0;

    // Assemble in two passes
    // Pass 1
    printf("Pass 1...\r\n");
    passInitialize(1);
    do {
        while(io_getline(FILE_CURRENT, line)) {
            strcpy(errorline, line);
            linenumber++;
            parseLine(line);
            if(!currentline.current_macro) {
                processInstructions();
                processDelayedLineNumberReset();
            }
            else processMacro();
            if(global_errors) {
                vdp_set_text_colour(DARK_YELLOW);
                printf("%s\r\n",errorline);
                vdp_set_text_colour(BRIGHT_WHITE);
                return false;
            }
        }
        if(inConditionalSection != 0) error(message[ERROR_MISSINGENDIF]);

        if(filestackCount()) {
            currentExpandedMacro = NULL;
            fclose(filehandle[FILE_CURRENT]);
            incfileState = filestackPop(&fsitem);
            io_setCurrent(&fsitem);
        }
        else incfileState = false;
    } while(incfileState);

    if(global_errors) return false;

    // Pass 2
    printf("Pass 2...\r\n");
    passInitialize(2);
    if(consolelist_enabled || list_enabled) {
        listInit();
    }
    readAnonymousLabel();
    
    do {
        while(io_getline(FILE_CURRENT, line)) {
            strcpy(errorline, line);
            linenumber++;
            if(consolelist_enabled || list_enabled) listStartLine(line);
            parseLine(line);
            if(!currentline.current_macro) {
                processInstructions();
                if(consolelist_enabled || list_enabled) listEndLine();
                processDelayedLineNumberReset();
            }
            else {
                if(consolelist_enabled || list_enabled) listEndLine();
                processMacro();
            }
            if(global_errors) {
                vdp_set_text_colour(DARK_YELLOW);
                printf("%s\r\n",errorline);
                vdp_set_text_colour(BRIGHT_WHITE);
                return false;
            }    
        }
        sourcefilecount++;
        if(filestackCount()) {
            currentExpandedMacro = NULL;
            fclose(filehandle[FILE_CURRENT]);
            incfileState = filestackPop(&fsitem);
            io_setCurrent(&fsitem);
        }
        else incfileState = false;
    } while(incfileState);

    printf("Test takes %.2f seconds\r\n",((double)(total) / CLOCKS_PER_SEC));
    return true;
}
