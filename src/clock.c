#include "clock.h"
#include <stdio.h>
#include <time.h>

//#ifndef AGON
clock_t begin, end;
//#endif

void clock_start(void) {
//#ifdef AGON
//	timer1_begin(10); // 10ms interval
//#else
    begin = clock();
//#endif
}

void clock_stop(void) {
//#ifdef AGON
//	timer1_end();
//#else
	end = clock();
//#endif
}

void clock_print(void) {
//#ifdef AGON
//	printf("%.2f seconds\r\n", ((double)(timer1)/100));
//#else
    printf("%.1f seconds\r\n",((double)(end - begin) / CLOCKS_PER_SEC));
//#endif
}
