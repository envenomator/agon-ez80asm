#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "label.h"
#include "hash.h"
#include "str2num.h"
#include "utils.h"
#include "globals.h"

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
void printLocalLabels(void) {
    uint8_t i;
    printf("Local label table:\n");
    if(localLabelCounter == 0) printf("Empty\n");
    for(i = 0; i < localLabelCounter; i++) printf("%s:%08x\n", localLabelTable[i].name, localLabelTable[i].address);
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
    //int i;

    //for(i = 0; i < localLabelCounter; i++) {
    //    localLabelTable[i].name[0] = 0;
    //}
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

void writeLocalLabels(FILE *fp) {
    int i;
    uint16_t delta;
    // number of bytes in the string buffer
    fwrite(&localLabelBufferIndex, 1, sizeof(localLabelBufferIndex), fp);
    // the actual bytes from the string buffer
    if(localLabelBufferIndex) fwrite(localLabelBuffer, 1, localLabelBufferIndex, fp);
    // the number of labels
    fwrite(&localLabelCounter, 1, sizeof(localLabelCounter), fp);
    for(i = 0; i < localLabelCounter; i++) {
        delta = localLabelTable[i].name - localLabelBuffer;
        fwrite(&delta, 1, sizeof(delta), fp);
        fwrite(&localLabelTable[i].address, 1, sizeof(uint32_t), fp);
    }
    //printf("LOCALS WRITTEN: %d\n", localLabelCounter);
}

void readLocalLabels(FILE *fp) {
    int i;
    uint16_t delta;

    fread(&localLabelBufferIndex, sizeof(localLabelBufferIndex), 1, fp);
    if(localLabelBufferIndex) fread(&localLabelBuffer, localLabelBufferIndex, 1, fp);
    fread(&localLabelCounter, sizeof(localLabelCounter), 1, fp);
    for(i = 0; i < localLabelCounter; i++) {
        fread(&delta, sizeof(delta), 1, fp);
        fread(&localLabelTable[i].address, sizeof(uint32_t), 1, fp);
        localLabelTable[i].name = localLabelBuffer + delta;
    }
    //printf("LOCALS READ: %d\n",localLabelCounter);
}

void writeAnonymousLabel(uint32_t address) {
    fwrite(&address, 1, sizeof(address), anonlabels);
}

void readAnonymousLabel(void) {
    uint32_t address;

    if(fread(&address, sizeof(address), 1, anonlabels)) {
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

bool insertLocalLabel(char *labelname, uint32_t address) {
    int len,i;
    char *ptr;
    char *old_name;
    uint32_t old_address;

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
        else error("Maximum number of local labels reached");
    }
    else error("Label already defined");
    return false;
}

bool insertGlobalLabel(char *labelname, uint32_t address){
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

uint16_t globalLabelTable_count() {
    return globalLabelCounter;
}

void print_globalLabelTable(){
    int i;
    printf("DEBUG - index\tname\taddress\n");
    for(i = 0; i < GLOBAL_LABEL_TABLE_SIZE; i++){
        if(globalLabelTable[i] != NULL) {
            printf("DEBUG - %i\t%s\t%u\n",i,globalLabelTable[i]->name,globalLabelTable[i]->address);
        }
    }
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
            strncmp(globalLabelTable[try]->name,name,GLOBAL_LABEL_TABLE_SIZE) == 0){
            return globalLabelTable[try];
        }
    }
    return NULL;
}

label *findLabel(char *name) {
    if(name[0] == '@') {
        
        if((name[1] == 'f') || (name[1] == 'F')) {
            if(an_next.defined) {
                an_return.address = an_next.address;
                return &an_return;
            }
            else return NULL;
        }
        if((name[1] == 'b') || (name[1] == 'B')) {
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