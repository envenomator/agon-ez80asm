; Test conditional EQU

    .if 1
test: EQU 1
    .else
test: EQU 2 ; will produce an error when not implemented
    .endif

    .db test
