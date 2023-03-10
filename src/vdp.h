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
void  vdp_fgcolour(unsigned char r, unsigned char g, unsigned char b);
void  vdp_bgcolour(unsigned char r, unsigned char g, unsigned char b);

#endif //VDP_H
