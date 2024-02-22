#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "label.h"
#include "hash.h"
#include "str2num.h"
#include "utils.h"
#include "globals.h"
#include "filestack.h"
#include "io.h"

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
        error("Couldn't open file for writing global label table");
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
    int i;

    labelmemsize = 0;
    globalLabelCounter = 0;
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++){
        globalLabelTable[i] = NULL;
    }
}

void initAnonymousLabelTable(void) {
    an_prev.defined = false;
    an_next.defined = false;
    an_return.name = NULL;
}

void initLocalLabelTable(void) {
    int i;

    localLabelBufferIndex = 0;
    localLabelCounter = 0;
    for(i = 0; i < LOCAL_LABEL_TABLE_SIZE; i++) {
        localLabelTable[i].name = NULL;
    }
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

label_t * findLocalLabel(char *key){
    int p = findLocalLabelIndex(key);
    if(p < LOCAL_LABEL_TABLE_SIZE) return &localLabelTable[p];
    else return NULL;
}

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
    int index,i,try,len;
    label_t *tmp;

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
    index = hash(labelname) % GLOBAL_LABEL_TABLE_SIZE;
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++) {
        try = (i + index) % GLOBAL_LABEL_TABLE_SIZE;
        if(globalLabelTable[try] == NULL){
            globalLabelTable[try] = tmp;
            globalLabelCounter++;
            return true;
        }
        if(i_strcasecmp(globalLabelTable[try]->name, tmp->name) == 0) {
            error(message[ERROR_LABELDEFINED]);
            return false;
        } 
    }
    return false;
}

label_t * findGlobalLabel(char *name){
    int index,i,try;

    index = hash(name) % GLOBAL_LABEL_TABLE_SIZE;
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++){
        try = (index + i) % GLOBAL_LABEL_TABLE_SIZE;
        if(globalLabelTable[try] == NULL){
            return NULL;
        }
        if(globalLabelTable[try] != NULL &&
            strcmp(globalLabelTable[try]->name,name) == 0){
            return globalLabelTable[try];
        }
    }
    return NULL;
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