#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "label.h"
#include "hash.h"
#include "str2num.h"
#include "utils.h"
#include "globals.h"

#define LABELBUFFERSIZE 131072

// memory buffer for sequentially storing label strings
char labelbuffer[LABELBUFFERSIZE];
uint16_t labelbufferindex;

// local label space
local_labels localLabels;

// table
label* label_table[LABEL_TABLE_SIZE];
uint16_t label_table_counter;
label* tmplocal;

void print_localLabels(void) {
    uint8_t i;
    printf("Local label table:\n");
    if(localLabels.number == 0) printf("Empty\n");
    for(i = 0; i < localLabels.number; i++) printf("@%d:%08x\n", i, localLabels.id[i]);
}

void clear_localLabels(void) {
    localLabels.number = 0;
}

void init_label_table(void) {
    int i;

    labelbufferindex = 0;
    label_table_counter = 0;
    for(i = 0; i < LABEL_TABLE_SIZE; i++){
        label_table[i] = NULL;
    }

    clear_localLabels();
    label_table_insert("TMPLOCAL", 0);
    tmplocal = label_table_lookup("TMPLOCAL");
}

void flush_locallabels(void) {
    if(localLabels.number) {
        write_localLabels(locals);
    }
}

void write_localLabels(FILE *fp) {
    uint8_t i;

    fwrite(&localLabels.number, 1, sizeof(localLabels.number), fp);
    for(i = 0; i < localLabels.number; i++) fwrite(&localLabels.id[i], 1, sizeof(localLabels.id[0]), fp);
    printf("LOCALS WRITTEN: %d\n", localLabels.number);
}

void read_localLabels(FILE *fp) {
    uint8_t i;
    fread(&localLabels.number, sizeof(localLabels.number), 1, fp);
    for(i = 0; i < localLabels.number; i++) fread(&localLabels.id[i], sizeof(localLabels.id[0]), 1, fp);
    printf("LOCALS READ: %d\n",localLabels.number);
    print_localLabels();
}

bool label_table_insert(char *labelname, uint32_t address){
    int index,i,try,len;
    label *tmp;

    if(labelname[0] == '@') { // local label
        labelname++;
        i = str2num(labelname);
        if(i > localLabels.number) {
            error("Local label defined out of sequence");
            return false;
        }
        if(i < localLabels.number) {
            error("Redefined local label");
            return false;
        }
        if(i >= LOCAL_LABELS) {
            error("Maximum number of local labels reached");
            return false;
        }
        localLabels.id[i] = address;
        localLabels.number++;
        return true;
    }
    len = strlen(labelname);
    // check space first
    if((labelbufferindex + len + 1 + sizeof(label)) > LABEL_TABLE_SIZE-1)
        return false; // no more space in buffer 
    // allocate space in buffer for label struct
    tmp = (label *)&labelbuffer[labelbufferindex];
    labelbufferindex += sizeof(label);
    // allocate space in buffer for string and store it to buffer
    tmp->name = &labelbuffer[labelbufferindex];
    labelbufferindex += len+1;
    strcpy(tmp->name, labelname);
    tmp->address = address;

    index = hash(labelname, LABEL_TABLE_SIZE);
    for(i = 0; i < LABEL_TABLE_SIZE; i++) {
        try = (i + index) % LABEL_TABLE_SIZE;
        if(label_table[try] == NULL){
            label_table[try] = tmp;
            label_table_counter++;
            return true;
        } 
    }
    return false;
}

uint16_t label_table_count() {
    return label_table_counter;
}

void print_label_table(){
    int i;
    printf("DEBUG - index\tname\taddress\n");
    for(i = 0; i < LABEL_TABLE_SIZE; i++){
        if(label_table[i] != NULL) {
            printf("DEBUG - %i\t%s\t%u\n",i,label_table[i]->name,label_table[i]->address);
        }
    }
}

void print_bufferspace(){
    printf("%d bytes available in label buffer\n", LABELBUFFERSIZE - labelbufferindex );
}

bool isglobalLabel(char *name) {
    return !(name[0] == '@');
}

label * label_table_lookup(char *name){
    int index,i,try;

    if(name[0] == '@') {
        name++;
        i = str2num(name);
        if((localLabels.number == 0) || (i > localLabels.number)) {
            error("Local label undefined");
            return NULL;
        }
        tmplocal->address = localLabels.id[i];
        return tmplocal;

    }
    index = hash(name, LABEL_TABLE_SIZE);
    for(i = 0; i < LABEL_TABLE_SIZE; i++){
        try = (index + i) % LABEL_TABLE_SIZE;
        if(label_table[try] == NULL){
            return NULL;
        }
        if(label_table[try] != NULL &&
            strncmp(label_table[try]->name,name,LABEL_TABLE_SIZE) == 0){
            return label_table[try];
        }
    }
    return NULL;
}