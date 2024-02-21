section .text
public _removefile
public _getfilesize

; Removes filename from SD card
; Input: filename string
; Output: A: File error, or 0 if OK
_removefile:
	push	ix
	ld 		ix,0
	add 	ix, sp
	ld 		hl, (ix+6)	; Address of path (zero terminated)
	ld a,	05h			; MOS_DEL API
	rst.lil	08h			; Delete a file or folder from the SD card
	ld		sp,ix
	pop		ix
	ret

; Gets filesize from an open file
; requires MOS mos_getfil call
; Input: MOS filehandle
; Output: HL - 24bit filesize

_getfilesize:
	PUSH IX
	LD	 IX, 0h
	ADD	 IX, SP

    ; get pointer to FIL structure in MOS memory
	LD	A, (IX+6)
    LD  C, A
    LD  A,	19h		; MOS_GETFIL API call
    RST.LIL 08h

    LD  DE, 11    ; offset to lower 3bytes in FSIZE_t, part of the FFOBJD struct that HL points to
    ADD HL, DE
    LD  HL, (HL)     ; load actual FSIZE_t value (lower 3 bytes)

	LD	SP, IX
	POP	IX
	RET
