#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define GLOBAL_LABEL_BUFFERSIZE 131072
#define LOCAL_LABEL_BUFFERSIZE   65536
#define GLOBAL_LABEL_TABLE_SIZE   8192
#define LOCAL_LABEL_TABLE_SIZE      64

typedef struct {
    char *name;
    uint32_t address;
} label;

typedef struct {
    bool defined;
    uint32_t address;
} anonymouslabeltype;

label *findLabel(char *name);
void initGlobalLabelTable(void);
void initAnonymousLabelTable(void);
bool insertGlobalLabel(char *labelname, uint32_t address);
bool insertLocalLabel(char *labelname, uint32_t address);
void clearLocalLabels(void);
void writeLocalLabels();
void readLocalLabels();
void writeAnonymousLabel(uint32_t address);
void readAnonymousLabel(void);
uint16_t label_table_count();
void print_label_table();
void printLocalLabels(void);
label * findGlobalLabel(char *name);
void initLocalLabelTable(void);
uint16_t getGlobalLabelCount(void);
uint16_t getLocalLabelCount(void);

// debug
void print_bufferspace();
#endif // LABEL_H
