#include "label.h"

// Total allocated memory for labels
uint24_t labelmemsize;

// memory for anonymous labels
anonymouslabel_t an_prev;
anonymouslabel_t an_next;
label_t an_return;

char localLabelBuffer[LOCAL_LABEL_BUFFERSIZE];
uint16_t localLabelBufferIndex;

// tables
label_t* globalLabelTable[GLOBAL_LABEL_TABLE_SIZE]; // hash table
uint16_t globalLabelCounter;
label_t localLabelTable[LOCAL_LABEL_TABLE_SIZE]; // indexed table
uint16_t localLabelCounter;

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
uint16_t getLocalLabelCount(void) {
    return localLabelCounter;
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

void initLocalLabelTable(void) {
    localLabelBufferIndex = 0;
    localLabelCounter = 0;
    memset(localLabelTable, 0, sizeof(localLabelTable));
}

void clearLocalLabels(void) {
    localLabelBufferIndex = 0;
    localLabelCounter = 0;
}

int findLocalLabelIndex(char *key) {
    int base = 0;
	int lim, cmp;
    int p;

	for (lim = localLabelCounter; lim != 0; lim >>= 1) {
		p = base + (lim >> 1);
		cmp = strcmp(key,localLabelTable[p].name);
		if (cmp == 0)
			return p;
		if (cmp > 0) {
			base = p + 1;
			lim--;
		}
	}
	return (LOCAL_LABEL_TABLE_SIZE);
}

label_t * findLocalLabel(char *key) {
    char compoundname[(MAXNAMELENGTH * 2)+1];

    if(filelabelscope[0] == 0) {
        // local to file label
        strcpy(compoundname, filename[FILE_CURRENT]);
    }
    else {
        // local to global label
        strcpy(compoundname, filelabelscope[FILE_CURRENT]);
    } // append local label
    strcat(compoundname, key);

    //printf("DEBUG: finding local label <%s>\r\n",compoundname);
    return findGlobalLabel(compoundname);
}

/*
label_t * findLocalLabel(char *key){
    int p = findLocalLabelIndex(key);
    if(p < LOCAL_LABEL_TABLE_SIZE) return &localLabelTable[p];
    else return NULL;
}
*/

void writeLocalLabels(void) {
    // the number of labels
    fwrite((char *)&localLabelCounter, sizeof(localLabelCounter), 1, filehandle[FILE_LOCAL_LABELS]);
    if(localLabelCounter) {
        fwrite((char *)&localLabelBufferIndex, sizeof(localLabelBufferIndex), 1, filehandle[FILE_LOCAL_LABELS]);
        // the actual bytes from the string buffer
        if(localLabelBufferIndex) 
            fwrite((char *)localLabelBuffer, localLabelBufferIndex, 1, filehandle[FILE_LOCAL_LABELS]);
        // the label table
        fwrite((char *)localLabelTable, localLabelCounter * sizeof(label_t), 1, filehandle[FILE_LOCAL_LABELS]);
    }
}

void readLocalLabels(void) {
    // the number of labels
    fread((char*)&localLabelCounter, sizeof(localLabelCounter), 1, filehandle[FILE_LOCAL_LABELS]);
    if(localLabelCounter) {
        fread((char*)&localLabelBufferIndex, sizeof(localLabelBufferIndex), 1, filehandle[FILE_LOCAL_LABELS]);
        if(localLabelBufferIndex) 
            fread((char*)&localLabelBuffer, localLabelBufferIndex, 1, filehandle[FILE_LOCAL_LABELS]);
        fread((char *)localLabelTable, localLabelCounter * sizeof(label_t), 1, filehandle[FILE_LOCAL_LABELS]);
    }
    else {
        localLabelBufferIndex = 0;
    }
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
        strcpy(compoundname, filename[FILE_CURRENT]);
    }
    else {
        // local to global label
        strcpy(compoundname, filelabelscope[FILE_CURRENT]);
    } // append local label
    strcat(compoundname, labelname);

    return insertGlobalLabel(compoundname, labelAddress);
}

bool insertLocalLabel_old(char *labelname, int24_t labelAddress) {
    int len,i;
    int p;
    char *ptr;
    char *old_name;
    int24_t old_address;

    if(labelname[1] == 0) {
        error(message[ERROR_INVALIDLABEL]);
        return false;
    }
    p = findLocalLabelIndex(labelname);

    if(p >= LOCAL_LABEL_TABLE_SIZE) {
        if(localLabelCounter < LOCAL_LABEL_TABLE_SIZE) {
            len = strlen(labelname);
            // check space first
            if((localLabelBufferIndex + len + 1) > LOCAL_LABEL_BUFFERSIZE -1)
                return false; // no more space in buffer
            // allocate space in buffer for string
            ptr = &localLabelBuffer[localLabelBufferIndex];
            localLabelBufferIndex += len + 1;
            strcpy(ptr, labelname);

            for(i = 0; i < localLabelCounter; i++) {
                if(strcmp(labelname, localLabelTable[i].name) < 0) break;
            }
            if(i < localLabelCounter) {
                for(; i < localLabelCounter; i++ ) {
                    old_name = localLabelTable[i].name;
                    old_address = localLabelTable[i].address;
                    localLabelTable[i].name = ptr;
                    localLabelTable[i].address = labelAddress;
                    ptr = old_name;
                    labelAddress = old_address;
                }
            }
            localLabelTable[localLabelCounter].name = ptr;
            localLabelTable[localLabelCounter].address = labelAddress;
            localLabelCounter++;
            return true;
        }
        else error(message[ERROR_MAXLOCALLABELS]);
    }
    else error(message[ERROR_LABELDEFINED]);
    return false;
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

void advanceLocalLabel(void) {
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
        writeLocalLabels();
        clearLocalLabels();

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

void refreshlocalLabels(void) {
    if((pass == 2) && (currentline.label) && (currentline.label[0] != '@')) {
        clearLocalLabels();
        readLocalLabels();
    }
}
