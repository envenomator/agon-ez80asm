true: .equ 1
false: .equ 0

proc:
.if true             ; this block will be selected
    ld hl, localvar  ; loading the address of the label in this block
    ld a, (hl)       ; A will be loaded with 1
    ret
variable:
localvar: .db 1
.else
    ld hl, localvar
    ld a, (hl)
    ret
    .asciz "This should not be here"
variable:
localvar: .db 0     ; the same label can be used, because this entire block will be skipped
.endif

procfalse:
.if false           ; this block will NOT be selected
    ld hl, localvar2; loading the address of the label in this block
    ld a, (hl)      ; A will be loaded with 1
    ret
variable2:
localvar2: .db 1
.else
    ld hl, localvar2
    ld a, (hl)
    ret
    .asciz "This should be here"
variable2:
localvar2: .db 0      ; the same label can be used, because this entire block will be skipped
.endif

    ld b, a         ; first regular statement
