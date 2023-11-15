#ifndef CLOCK_H
#define CLOCK_H

#include "config.h"

#ifdef AGON
#include "agontimer.h"
#else
#include <time.h>
#endif

void clock_start(void);
void clock_stop(void);
void clock_print(void);

#endif