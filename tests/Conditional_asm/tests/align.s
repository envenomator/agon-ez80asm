; Test alignment
; Don't specifically test else statement; if alignment doesn't respect if/else, then the output will be different
    .db 0
    .if 1
    .align 16
    .else
    .align 32
    .endif

    .db 0
