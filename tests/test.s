    include "tests/test.inc"
start:
    ld a,b
@@:
    ld hl, @p

    sum 10,15
    
    ld a,b
