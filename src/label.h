#ifndef LABEL_H
#define LABEL_H

#include <stdbool.h>
#include <stdint.h>

#define LABEL_TABLE_SIZE 8192

typedef struct {
    char *name;
    uint32_t address;
} label;

void init_label_table();
bool label_table_insert(char *labelname, uint32_t address);
uint16_t label_table_count();
void print_label_table();
label * label_table_lookup(char *name);

// debug
void print_bufferspace();
#endif // LABEL_H
