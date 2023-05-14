; line with label, opcode, tokens
; objective: check parser handling of spacing and parsing of basic tokens
;
; regular spaces, without comments
label1:ld a ,b
;label2:ld.lil a,(5)
label3: ld a,b
label4: ld a, b
label5: ld a, b
label6: ld a, b
; with tabs
label7:   ld  a   ,   b
; with comments
label8: ld a,b;comment
label9: ld a,b ;comment
labela: ld a,b ; comment
; empty line

; simple opcodes spaced between labels
labelb:
    ld a,b
    ld b,c
labelc:
    ld a,b
    ld b,c

; register indirect with spaces
    ld a,(bc)
    ld a,(de)
    ld a,(hl)
    ld a, (hl)
    ld a ,(hl)
    ld a , (hl)
    ld a, ( bc)
    ld a, ( de)
    ld a, ( hl)
    ld a, (bc )
    ld a, (de )
    ld a, (hl )
    ld a, ( bc )
    ld a, ( de )
    ld a, ( hl )
    ld a,(ix+1)
    ld a,( ix+1)
    ld a,( ix+1 )
    ld a,(ix+1 )
    ld a,(ix+ 1)
    ld a,(ix +1)
    ld a,(ix + 1)
    ld a,( ix + 1)

    ld a,(iy+1) ; iy has separate parsing code
    ld a,( iy+1)
    ld a,( iy+1 )
    ld a,(iy+1 )
    ld a,(iy+ 1)
    ld a,(iy +1)
    ld a,(iy + 1)
    ld a,( iy + 1)

; register indirect with tabs
    ld	a,(hl)
    ld	a,	(hl)
    ld	a	,(hl)
    ld	a	,	(hl)
    ld	a,	(	bc)
    ld	a,	(	de)
    ld	a,	(	hl)
    ld	a,	(bc	)
    ld	a,	(de	)
    ld	a,	(hl	)
    ld	a,	(	bc	)
    ld	a,	(	de	)
    ld	a,	(	hl	)
    ld a,(ix+1)
    ld a,(	ix+1)
    ld a,(	ix+1	)
    ld a,(ix+1	)
    ld a,(ix+	1)
    ld a,(ix	+1)
    ld a,(ix	+	1)
    ld a,(	ix	+	1)

    ld a,(iy+1) ; iy has separate parsing code
    ld a,(	iy+1)
    ld a,(	iy+1	)
    ld a,(iy+1	)
    ld a,(iy+	1)
    ld a,(iy	+1)
    ld a,(iy	+	1)
    ld a,(	iy	+	1)

; simple number parsing with spaces and tabs
    ld a, ffh
    ld a, ffh 
    ld a, ffh ; with comment
    ld a,	ffh
    ld a,	ffh	
    ld a,	ffh	; with comment

; simple indirect number parsing with spaces and tabs
    ld (50000h),a
    ld (50000h), a
    ld (50000h),	a
    ld (50000h) ,a
    ld (50000h)	,a
    ld ( 50000h),a
    ld (	50000h),a
    ld ( 50000h ),a
    ld (	50000h	),a
