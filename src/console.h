#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>
#include "globals.h"

#ifndef CEDEV
void vdp_set_text_colour( int colour );
#else
#include <agon/vdp_vdu.h>
#endif


#endif