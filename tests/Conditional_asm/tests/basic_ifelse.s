; Test both if/else code blocks
; Also test both basic assembly emition and emition of data/blk directives

TRUE:   EQU 1
FALSE:  EQU 0

; Test if statement
.if TRUE
    ld a,1
    .db 1
    .blkb 1,1
.else
    ld a,2
    .db 2
    .blkb 1,2
.endif

; Test else statement
.if FALSE
    ld a,3
    .db 3
    .blkb 1,3
.else
    ld a,4
    .db 4
    .blkb 1,4
.endif
