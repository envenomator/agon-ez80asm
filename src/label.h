#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "typedefs.h"
#include "label.h"
#include "hash.h"
#include "str2num.h"
#include "utils.h"
#include "globals.h"
#include "io.h"
#include "macro.h"

label_t *findLabel(const char *name);
void initGlobalLabelTable(void);
void initAnonymousLabelTable(void);
void writeAnonymousLabel(uint24_t address);
void readAnonymousLabel(void);
label_t * findGlobalLabel(const char *name);
uint16_t getGlobalLabelCount(void);
void saveGlobalLabelTable(void);
void advanceAnonymousLabel(void);
void definelabel(uint24_t num);

extern uint24_t labelmemsize;

#endif // LABEL_H
