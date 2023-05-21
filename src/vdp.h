/*
 * Title:			AGON VDP - VDP c header interface
 * Author:			Jeroen Venema
 * Created:			20/10/2022
 * Last Updated:	20/10/2022
 * 
 * Modinfo:
 * 20/10/2022:		Initial version: text/graphics functions
 * 22/10/2022:		Bitmap/Sprite functions added
 */

#ifdef AGON
#include <defines.h>
#endif
#include "mos-interface.h"

#ifndef VDP_H
#define VDP_H

// DEFAULT COLOR INDEXES
enum {
    BLACK = 0,
    DARK_RED,
    DARK_GREEN,
    DARK_YELLOW,
    DARK_BLUE,
    DARK_MAGENTA,
    DARK_CYAN,
    DARK_WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE
};

// VDP modes
#define VDPMODE_1024x728_2C 0
#define VDPMODE_512x384_16C 1
#define VDPMODE_320x200_64C 2
#define VDPMODE_640x480_16C 3
#define VDPMODE_DEFAULT     1

// Generic
void vdp_mode(unsigned char mode);
//
// extent: 0 = current text window, 1 = entire screen
// direction: 0 = right, 1 = left, 2 = down, 3 = up
// speed: number of pixels to scroll
void vdp_scroll(unsigned char extent, unsigned char direction, unsigned char speed);

// Text VDP functions
void  vdp_cls();
void  vdp_cursorHome();
void  vdp_cursorUp();
void  vdp_cursorGoto(unsigned char x, unsigned char y);
void  vdp_fgcolour(unsigned char colorindex);
void  vdp_bgcolour(unsigned char colorindex);

#endif //VDP_H
