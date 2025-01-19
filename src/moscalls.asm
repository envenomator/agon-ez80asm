assume adl=1
section .text
public _removefile
public _getfilesize
public _fast_strcasecmp
public _fast_strncasecmp

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

; Fast lowercase compare between two strings
; Stops at first difference of the two strings, or at either string terminator (0)
; returns first character difference in A
_fast_strcasecmp:
        push    ix
        ld      ix,0
        add     ix,sp
        dec     sp

		ld		hl, (ix+6)	; s1
		ld		de, (ix+9)	; s2
.loop:
        ld      c,(hl)		; *s1
        inc     hl			; s1++

		; *s1 tolower
        ld      a, c
        sub     a, 'A'
        cp      a, 1+'Z'-'A'
        jr      nc, .conts2
        add     a, 'a'
        ld      c, a
.conts2: ; c now contains tolower(*s1)

        ex		de, hl
		ld      b,(hl)		; *s2
        inc     hl			; s2++
		ex		de, hl

		; *s2 tolower
		ld		a, b
        sub     a, 'A'
        cp      a, 1+'Z'-'A'
        jr      nc, .conts1
        add     a, 'a'
		ld		b, a
.conts1: ; d now contains tolower(*s2)
		ld		a, b
		cp		a, 0		; stop at 0 of string s2
		jr		z, .done
		cp		a, c		; loop back, or stop at difference (might be 0 of string s1)
		jr		z, .loop

.done:
		sub		a, c		; *s1 == *s2? -> result in A (0 == equal)
        ld      sp,ix
        pop     ix
        ret


; Fast lowercase compare between two strings, max uint8_t n characters
; Stops at first difference of the two strings, or at either string terminator (0)
; returns first character difference in A
_fast_strncasecmp:
        push    ix
        ld      ix,0
        add     ix,sp
        dec     sp

		ld		hl, (ix+12) ; n
		add		hl, de
		or		a,a
		sbc		hl,de
		jr		z, .ret0	; n == 0? return 0

		ld		hl, (ix+6)	; s1
		ld		de, (ix+9)	; s2

.loop:
        ld      c,(hl)		; *s1
        inc     hl			; s1++

		; *s1 tolower
        ld      a, c
        sub     a, 'A'
        cp      a, 1+'Z'-'A'
        jr      nc, .conts2
        add     a, 'a'
        ld      c, a
.conts2: ; c now contains tolower(*s1)

        ex		de, hl
		ld      b,(hl)		; *s2
        inc     hl			; s2++
		ex		de, hl

		; *s2 tolower
		ld		a, b
        sub     a, 'A'
        cp      a, 1+'Z'-'A'
        jr      nc, .conts1
        add     a, 'a'
		ld		b, a
.conts1: ; d now contains tolower(*s2)
		ld		a, b
		cp		a, 0		; stop at 0 of string s2
		jr		z, .done
		cp		a, c		; loop back, or stop at difference (might be 0 of string s1)
		jr		z, .checkn0

.done:
		sub		a, c		; *s1 == *s2? -> result in A (0 == equal)
        ld      sp,ix
        pop     ix
        ret

.ret0:
		ld		a,0
		ld		sp, ix
		pop		ix
		ret

.checkn0:
		push	hl
		ld		hl, (ix+12)
		dec		hl
		ld		(ix+12), hl
		add		hl, de
		or		a,a
		sbc		hl,de
		pop		hl
		jr		z, .ret0	; n == 0? return 0

		jr		.loop
