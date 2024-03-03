#include "label.h"

// Total allocated memory for labels
uint24_t labelmemsize;

// memory for anonymous labels
anonymouslabel_t an_prev;
anonymouslabel_t an_next;
label_t an_return;

// tables
label_t* globalLabelTable[GLOBAL_LABEL_TABLE_SIZE]; // hash table
uint16_t globalLabelCounter;

void saveGlobalLabelTable(void) {
    int i;
    char *ptr;
    char buffer[LINEMAX];

    filehandle[FILE_SYMBOLS] = fopen(filename[FILE_SYMBOLS], "wb+");
    if(filehandle[FILE_SYMBOLS] == 0) {
        error(message[ERROR_FILEGLOBALLABELS]);
        return;
    }

    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++) {
        if(globalLabelTable[i]) {
            sprintf(buffer, "%s $%x\r\n", globalLabelTable[i]->name, globalLabelTable[i]->address);
            ptr = buffer;
            while(*ptr) fputc(*ptr++, filehandle[FILE_SYMBOLS]);
        }
    }
    fclose(filehandle[FILE_SYMBOLS]);
}

uint16_t getGlobalLabelCount(void) {
    return globalLabelCounter;
}

void initGlobalLabelTable(void) {
    labelmemsize = 0;
    globalLabelCounter = 0;
    labelcollisions = 0;
    memset(globalLabelTable, 0, sizeof(globalLabelTable));
}

void initAnonymousLabelTable(void) {
    an_prev.defined = false;
    an_next.defined = false;
    an_return.name = NULL;
}

label_t * findLocalLabel(char *key) {
    char compoundname[(MAXNAMELENGTH * 2)+1];

    if(filelabelscope[0] == 0) {
        // local to file label
        strcompound(compoundname, filename[FILE_CURRENT], key);
    }
    else {
        // local to global label
        strcompound(compoundname, filelabelscope[FILE_CURRENT], key);
    }
    return findGlobalLabel(compoundname);
}

void writeAnonymousLabel(int24_t labelAddress) {
    uint8_t scope;

    scope = filestackCount();
    fwrite((char*)&labelAddress, sizeof(labelAddress), 1, filehandle[FILE_ANONYMOUS_LABELS]);
    fwrite((char*)&scope, sizeof(scope), 1, filehandle[FILE_ANONYMOUS_LABELS]);
    fflush(filehandle[FILE_ANONYMOUS_LABELS]);
}

void readAnonymousLabel(void) {
    int24_t labelAddress;
    uint8_t scope;

    if(fread((char*)&labelAddress, sizeof(labelAddress), 1, filehandle[FILE_ANONYMOUS_LABELS])) {
        fread((char*)&scope, sizeof(scope), 1, filehandle[FILE_ANONYMOUS_LABELS]);
        if(an_next.defined) {
            an_prev.address = an_next.address;
            an_prev.scope = an_next.scope;
            an_prev.defined = true;
        }
        an_next.address = labelAddress;
        an_next.scope = scope;            
        an_next.defined = true;
    }
    else { // last label already read
        an_prev.address = an_next.address;
        an_prev.scope = an_next.scope;
        an_prev.defined = true;
        an_next.defined = false;
    }
}

bool insertLocalLabel(char *labelname, int24_t labelAddress) {
    char compoundname[(MAXNAMELENGTH * 2)+1];

    if(filelabelscope[0] == 0) {
        // local to file label
        strcompound(compoundname, filename[FILE_CURRENT], labelname);
    }
    else {
        // local to global label
        strcompound(compoundname, filelabelscope[FILE_CURRENT], labelname);
    }
    return insertGlobalLabel(compoundname, labelAddress);
}

bool insertGlobalLabel(char *labelname, int24_t labelAddress){
    int index,len;
    label_t *tmp,*try;

    //printf("DEBUG: inserting label <%s>\r\n",labelname);
    len = strlen(labelname);

    // allocate space in buffer for label_t struct
    tmp = (label_t *)malloc(sizeof(label_t));
    labelmemsize += sizeof(label_t);
    if(tmp == 0) return false;

    // allocate space in buffer for string and store it to buffer
    tmp->name = (char*)malloc(len+1);
    labelmemsize += len+1;
    if(tmp->name == 0) return false;

    strcpy(tmp->name, labelname);
    tmp->address = labelAddress;
    tmp->next = NULL;

    index = hash(labelname) % GLOBAL_LABEL_TABLE_SIZE;
    try = globalLabelTable[index];

    // First item on index
    if(try == NULL) {
        globalLabelTable[index] = tmp;
        globalLabelCounter++;
        return true;
    }

    // Collision on index, place at end of linked list if unique
    while(true) {
        if(strcmp(try->name, labelname) == 0) {
            error(message[ERROR_LABELDEFINED]);
            return false;
        }
        labelcollisions++;
        if(try->next) {
            try = try->next;
        }
        else {
            try->next = tmp;
            globalLabelCounter++;
            return true;
        }
    }
}

label_t * findGlobalLabel(char *name){
    int index;
    label_t *try;

    index = hash(name) % GLOBAL_LABEL_TABLE_SIZE;
    try = globalLabelTable[index];

    while(true)
    {
        if(try == NULL) return NULL;
        if(strcmp(try->name, name) == 0) return try;
        try = try->next;
    }
}

label_t *findLabel(char *name) {
    if(name[0] == '@') {
        if(((tolower(name[1]) == 'f') || (tolower(name[1]) == 'n')) && name[2] == 0) {
            if(an_next.defined && an_next.scope == filestackCount()) {
                an_return.address = an_next.address;
                return &an_return;
            }
            else return NULL;
        }
        if(((tolower(name[1]) == 'b') || (tolower(name[1]) == 'p')) && name[2] == 0) {
            if(an_prev.defined && an_prev.scope == filestackCount()) {
                an_return.address = an_prev.address;
                return &an_return;
            }
            else return NULL;
        }
        return findLocalLabel(name);
    }
    else return findGlobalLabel(name);
}

void advanceAnonymousLabel(void) {
    if(currentline.label) {
        if(currentline.label[0] == '@') {
            if(currentline.label[1] == '@') {
                if(!recordingMacro) readAnonymousLabel();
            }
        }
    }
}

void definelabel(int24_t num){
    if(pass == 1) {
        if(currentline.label == NULL) return;

        if(currentline.label[0] == '@') {
            if(currentline.label[1] == '@') {
                writeAnonymousLabel(num);
                return;
            }
            if(insertLocalLabel(currentline.label, num) == false) {
                error(message[ERROR_CREATINGLABEL]);
                return;
            }
            return;
        }
        if(currentline.label[0] == '$') {
            error(message[ERROR_INVALIDLABEL]);
            return;
        }
        str2num(currentline.label, strlen(currentline.label)); 
        if(!err_str2num) { // labels can't have a valid number format
            error(message[ERROR_INVALIDLABEL]);
            return;
        }
        if(insertGlobalLabel(currentline.label, num) == false){
            error(message[ERROR_CREATINGLABEL]);
            return;
        }
        //writeLocalLabels();
        //clearLocalLabels();

        if(currentline.label) {
            strcpy(filelabelscope[FILE_CURRENT], currentline.label);
            //printf("DEBUG: changing scope to <%s> at address <0x%06X>\r\n",currentline.label, num);
        }

        return;
    }
    if(currentline.label && currentline.label[0] != '@') {
        strcpy(filelabelscope[FILE_CURRENT], currentline.label);
        //printf("DEBUG: changing scope to <%s> at address <0x%06X>\r\n",currentline.label, num);
    }
}
/*
void refreshlocalLabels(void) {
    if((pass == 2) && (currentline.label) && (currentline.label[0] != '@')) {
        clearLocalLabels();
        readLocalLabels();
    }
}
*/
