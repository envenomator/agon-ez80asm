#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "label.h"
#include "hash.h"
#include "str2num.h"
#include "utils.h"
#include "globals.h"
#include "stdint.h"

// memory for anonymous labels
anonymouslabeltype an_prev;
anonymouslabeltype an_next;
label an_return;

// memory buffer for sequentially storing label strings
char globalLabelBuffer[GLOBAL_LABEL_BUFFERSIZE];
char localLabelBuffer[LOCAL_LABEL_BUFFERSIZE];
uint16_t globalLabelBufferIndex;
uint16_t localLabelBufferIndex;

// tables
label* globalLabelTable[GLOBAL_LABEL_TABLE_SIZE];
uint16_t globalLabelCounter;
label localLabelTable[LOCAL_LABEL_TABLE_SIZE];
uint16_t localLabelCounter;

uint16_t getGlobalLabelCount(void) {
    return globalLabelCounter;
}
uint16_t getLocalLabelCount(void) {
    return localLabelCounter;
}
void initGlobalLabelTable(void) {
    int i;

    globalLabelBufferIndex = 0;
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

label * findLocalLabel(char *key){
    int p = findLocalLabelIndex(key);
    if(p < LOCAL_LABEL_TABLE_SIZE) return &localLabelTable[p];
    else return NULL;
}

void writeLocalLabels(void) {
    int i;
    uint16_t delta;
    // number of bytes in the string buffer
    agon_fwrite(&localLabelBufferIndex, 1, sizeof(localLabelBufferIndex), FILE_LOCAL_LABELS);
    // the actual bytes from the string buffer
    if(localLabelBufferIndex) agon_fwrite(localLabelBuffer, 1, localLabelBufferIndex, FILE_LOCAL_LABELS);
    // the number of labels
    agon_fwrite(&localLabelCounter, 1, sizeof(localLabelCounter), FILE_LOCAL_LABELS);
    for(i = 0; i < localLabelCounter; i++) {
        delta = localLabelTable[i].name - localLabelBuffer;
        agon_fwrite(&delta, 1, sizeof(delta), FILE_LOCAL_LABELS);
        agon_fwrite(&localLabelTable[i].address, 1, sizeof(int32_t), FILE_LOCAL_LABELS);
    }
}

void readLocalLabels(void) {
    int i;
    uint16_t delta;

    agon_fread(&localLabelBufferIndex, sizeof(localLabelBufferIndex), 1, FILE_LOCAL_LABELS);
    if(localLabelBufferIndex) agon_fread(&localLabelBuffer, localLabelBufferIndex, 1, FILE_LOCAL_LABELS);
    agon_fread(&localLabelCounter, sizeof(localLabelCounter), 1, FILE_LOCAL_LABELS);
    for(i = 0; i < localLabelCounter; i++) {
        agon_fread(&delta, sizeof(delta), 1, FILE_LOCAL_LABELS);
        agon_fread(&localLabelTable[i].address, sizeof(int32_t), 1, FILE_LOCAL_LABELS);
        localLabelTable[i].name = localLabelBuffer + delta;
    }
}

void writeAnonymousLabel(int32_t address) {
    agon_fwrite(&address, 1, sizeof(address), FILE_ANONYMOUS_LABELS);
}

void readAnonymousLabel(void) {
    int32_t address;

    if(agon_fread(&address, sizeof(address), 1, FILE_ANONYMOUS_LABELS)) {
        if(an_next.defined) {
            an_prev.address = an_next.address;
            an_prev.defined = true;
        }
        an_next.address = address;
        an_next.defined = true;            
    }
    else { // last label already read
        an_prev.address = an_next.address;
        an_prev.defined = true;
        an_next.defined = false;
    }
}

bool insertLocalLabel(char *labelname, int32_t address) {
    int len,i;
    char *ptr;
    char *old_name;
    int32_t old_address;

    if(labelname[1] == 0) {
        error(message[ERROR_INVALIDLABEL]);
        return false;
    }
    int p = findLocalLabelIndex(labelname);

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
                    localLabelTable[i].address = address;
                    ptr = old_name;
                    address = old_address;
                }
            }
            localLabelTable[localLabelCounter].name = ptr;
            localLabelTable[localLabelCounter].address = address;
            localLabelCounter++;
            return true;
        }
        else error(message[ERROR_MAXLOCALLABELS]);
    }
    else error(message[ERROR_LABELDEFINED]);
    return false;
}

bool insertGlobalLabel(char *labelname, int32_t address){
    int index,i,try,len;
    label *tmp;

    len = strlen(labelname);
    // check space first
    if((globalLabelBufferIndex + len + 1 + sizeof(label)) > GLOBAL_LABEL_TABLE_SIZE-1)
        return false; // no more space in buffer 
    // allocate space in buffer for label struct
    tmp = (label *)&globalLabelBuffer[globalLabelBufferIndex];
    globalLabelBufferIndex += sizeof(label);
    // allocate space in buffer for string and store it to buffer
    tmp->name = &globalLabelBuffer[globalLabelBufferIndex];
    globalLabelBufferIndex += len+1;
    strcpy(tmp->name, labelname);
    tmp->address = address;

    index = hash(labelname, GLOBAL_LABEL_TABLE_SIZE);
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++) {
        try = (i + index) % GLOBAL_LABEL_TABLE_SIZE;
        if(globalLabelTable[try] == NULL){
            globalLabelTable[try] = tmp;
            globalLabelCounter++;
            return true;
        } 
    }
    return false;
}

void print_bufferspace(){
    printf("%d bytes available in label buffer\n", GLOBAL_LABEL_BUFFERSIZE - globalLabelBufferIndex );
}

label * findGlobalLabel(char *name){
    int index,i,try;

    index = hash(name, GLOBAL_LABEL_TABLE_SIZE);
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

label *findLabel(char *name) {
    if(name[0] == '@') {
        
        if(((name[1] == 'f') || (name[1] == 'n')) && name[2] == 0) {
            if(an_next.defined) {
                an_return.address = an_next.address;
                return &an_return;
            }
            else return NULL;
        }
        if(((name[1] == 'b') || (name[1] == 'p')) && name[2] == 0) {
            if(an_prev.defined) {
                an_return.address = an_prev.address;
                return &an_return;
            }
            else return NULL;
        }
        
        return findLocalLabel(name);
    }
    else return findGlobalLabel(name);
}