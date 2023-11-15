/*
 * Title:			AGON timer interface
 * Author:			Jeroen Venema
 * Created:			06/11/2022
 * Last Updated:	22/01/2023
 * 
 * Modinfo:
 * 06/11/2022:		Initial version
 * 22/01/2023:		Interrupt-based freerunning functions added for timer1
 * 10/04/2023:		Using mos_setintvector
 */

#include "config.h"

#ifdef AGON
#include <defines.h>
#include <ez80.h>
#include "agontimer.h"
#include "mos-interface.h"

void *_timer1_prevhandler;						// used to (re)store the previous handler for the interrupt

// start timer1 on a millisecond interval
// this function registers an interrupt handler and requires timer1_end to de-register the handler after use
void timer1_begin(int interval)
{
	unsigned char tmp;
	unsigned short rr;
	
	_timer1_prevhandler = mos_setintvector(PRT1_IVECT, timer1_handler);

	timer1 = 0;
	TMR1_CTL = 0x00;
	rr = (unsigned short)(((18432000 / 1000) * interval) / 16);
	TMR1_RR_H = (unsigned char)(rr >> 8);
	TMR1_RR_L = (unsigned char)(rr);
	tmp = TMR1_CTL;
    TMR1_CTL = 0x57;
}

void timer1_end(void)
{
	TMR1_CTL = 0;
	mos_setintvector(PRT1_IVECT, _timer1_prevhandler);
}

#endif