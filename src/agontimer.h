/*
 * Title:			AGON timer interface
 * Author:			Jeroen Venema
 * Created:			06/11/2022
 * Last Updated:	22/01/2023
 * 
 * Modinfo:
 * 06/11/2022:		Initial version
 * 22/01/2023:      Freerunning timer0 code added, needs interrupt handler code
 */

#include "stdint.h"
#include "config.h"
#include <defines.h>

#ifndef AGONTIMER_H
#define AGONTIMER_H

#ifdef AGON

extern volatile uint24_t timer1;
extern void	timer1_handler(void);
void timer1_begin(int interval);
void timer1_end(void);

void delayms(int ms);
#endif

#endif //AGONTIMER_H
