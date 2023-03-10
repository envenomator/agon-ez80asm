#include "vdp.h"
#include "mos-interface.h"

// Generic functions

void vdp_mode(unsigned char mode)
{
    putch(22);
    putch(mode);
}

// Text functions
void vdp_cls()
{
    putch(12);
}

void vdp_cursorHome()
{
    putch(30);
}

void vdp_cursorUp()
{
    putch(11);
}

void vdp_cursorGoto(unsigned char x, unsigned char y)
{
    putch(31); // TAB
    putch(x);
    putch(y);
}

void vdp_colour(unsigned char foreground, unsigned char r, unsigned char g, unsigned char b)
{
    putch(17); // COLOUR
    putch(!foreground);
    putch(r);
    putch(g);
    putch(b);
}

void vdp_fgcolour(unsigned char r, unsigned char g, unsigned char b)
{
    vdp_colour(1,r,g,b);
}

void vdp_bgcolour(unsigned char r, unsigned char g, unsigned char b)
{
    vdp_colour(0,r,g,b);
}
