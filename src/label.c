#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "label.h"
#include "hash.h"

#define LABELBUFFERSIZE 65536

// memory buffer for sequentially storing label strings
char labelbuffer[LABELBUFFERSIZE];
uint16_t labelbufferindex;

// table
label* label_table[LABEL_TABLE_SIZE];
uint16_t label_table_counter;

void init_label_table() {
    int i;

    labelbufferindex = 0;
    label_table_counter = 0;
    for(i = 0; i < LABEL_TABLE_SIZE; i++){
        label_table[i] = NULL;
    }
}

bool label_table_insert(char *labelname, uint32_t address){
    int index,i,try,len;
    label *tmp;

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
    printf("index\tname\taddress\n");
    for(i = 0; i < LABEL_TABLE_SIZE; i++){
        if(label_table[i] != NULL) {
            printf("%i\t%s\t%u\n",i,label_table[i]->name,label_table[i]->address);
        }
    }
}

void print_bufferspace(){
    printf("%d bytes available in label buffer\n", LABELBUFFERSIZE - labelbufferindex );
}

label * label_table_lookup(char *name){
    int index,i,try;
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