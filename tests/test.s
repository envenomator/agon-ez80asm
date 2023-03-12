    include "tests/test.inc"
start:
    ld a,b
@@:
    ld hl, @p

    macro negative arg1,arg2
    ld a, arg1
    endmacro

    sum 10,15
    
    ld a,b
    negative 10,15

