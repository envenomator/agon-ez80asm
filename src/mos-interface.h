/*
 * Title:			AGON MOS - MOS c header interface
 * Author:			Jeroen Venema
 * Created:			15/10/2022
 * Last Updated:	15/10/2022
 * 
 * Modinfo:
 * 15/10/2022:		Added putch, getch
 * 22/10/2022:		Added waitvblank, mos_f* functions
 * 10/01/2023:      Added getsysvar_cursorX/Y functions, removed generic getsysvar8/16/24bit functions
 */

#ifndef MOS_H
#define MOS_H

#include <defines.h>

// File access modes - from mos_api.inc
#define fa_read				0x01
#define fa_write			0x02
#define fa_open_existing	0x00
#define fa_create_new		0x04
#define fa_create_always	0x08
#define fa_open_always		0x10
#define fa_open_append		0x30

// Indexes into sysvar - from mos_api.inc
#define sysvar_time			0x00
#define sysvar_vpd_pflags	0x04
#define sysvar_keycode		0x05
#define sysvar_keymods		0x06
#define sysvar_cursorX		0x07
#define sysvar_cursorY		0x08
#define sysvar_scrchar		0x09
#define sysvar_scrpixel		0x0A
#define sysvar_audioChannel	0x0D
#define syscar_audioSuccess	0x0E

extern int  putch(int a);
extern char  getch(void);
extern void  waitvblank(void);

extern UINT8 getsysvar_cursorX();
extern UINT8 getsysvar_cursorY();
extern UINT8 getsysvar_scrchar();

// MOS API calls
extern UINT8 mos_fopen(char * filename, UINT8 mode); // returns filehandle, or 0 on error
extern UINT8 mos_fclose(UINT8 fh);					 // returns number of still open files
extern char	 mos_fgetc(UINT8 fh);					 // returns character from file
extern void	 mos_fputc(UINT8 fh, char c);			 // writes character to file
extern UINT8 mos_feof(UINT8 fh);					 // returns 1 if EOF, 0 otherwise
extern UINT8 mos_save(char *filename, UINT24 address, UINT24 nbytes);
extern UINT8 mos_del(char *filename);

#endif MOS_H