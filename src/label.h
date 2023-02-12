#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define LABEL_TABLE_SIZE 8192
#define LOCAL_LABELS    2

typedef struct {
    char *name;
    uint32_t address;
} label;

typedef struct {
    uint8_t     number;
    uint32_t    id[LOCAL_LABELS];
} local_labels;

void init_label_table(void);
void print_localLabels(void);
void clear_localLabels(void);
void write_localLabels(FILE *fp);
void read_localLabels(FILE *fp);
bool isglobalLabel(char *name);
bool label_table_insert(char *labelname, uint32_t address);
uint16_t label_table_count();
void print_label_table();
label * label_table_lookup(char *name);

// debug
void print_bufferspace();
#endif // LABEL_H
