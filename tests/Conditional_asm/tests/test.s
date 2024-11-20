select: .equ 1

proc:
.if select           ; this block will be selected as 'select' is 1 
    ld hl, localvar  ; loading the address of the label in this block
    ld a, (hl)       ; A will be loaded with 1
    ret
variable:
localvar: .db 1
.else
    ld hl, localvar
    ld a, (hl)
    ret
    .asciz "Dummy data" ; This shouldn't produce a trailing zero
variable:            ; This shouldn't error
localvar: .db 0      ; the same label can be used, because this entire block will be skipped
.endif

    ld b, a         ; first regular statement
