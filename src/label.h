#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include "./stdint.h"
#include "config.h"
#include "malloc.h"

enum {
    LABEL_REGULAR,
    LABEL_MACRO
};

typedef struct {
    char *name;
    int24_t address;
} label;

typedef struct {
    bool defined;
    int24_t address;
} anonymouslabeltype;

label *findLabel(char *name);
void initGlobalLabelTable(void);
void initAnonymousLabelTable(void);
bool insertGlobalLabel(char *labelname, int24_t address);
bool insertLocalLabel(char *labelname, int24_t address);
void clearLocalLabels(void);
void writeLocalLabels(void);
void readLocalLabels(void);
void writeAnonymousLabel(int24_t address);
void readAnonymousLabel(void);
label * findGlobalLabel(char *name);
void initLocalLabelTable(void);
uint16_t getGlobalLabelCount(void);
uint16_t getLocalLabelCount(void);
void printLocalLabelTable(void);

#endif // LABEL_H
