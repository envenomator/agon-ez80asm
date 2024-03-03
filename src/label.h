#ifndef LABEL_H
#define LABEL_H

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

enum {
    LABEL_REGULAR,
    LABEL_MACRO
};

typedef struct {
    char *name;
    void *next;
    int24_t address;
} label_t;

typedef struct {
    uint8_t scope;
    bool defined;
    int24_t address;
} anonymouslabel_t;

label_t *findLabel(char *name);
void initGlobalLabelTable(void);
void initAnonymousLabelTable(void);
bool insertGlobalLabel(char *labelname, int24_t address);
void writeAnonymousLabel(int24_t address);
void readAnonymousLabel(void);
label_t * findGlobalLabel(char *name);
uint16_t getGlobalLabelCount(void);
void saveGlobalLabelTable(void);
void advanceAnonymousLabel(void);
void definelabel(int24_t num);

extern uint24_t labelmemsize;

#endif // LABEL_H
