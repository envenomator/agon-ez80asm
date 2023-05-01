    include "test.inc"

    macro defspace num,value
    ds num,value
    endmacro

    macro defdbtest idx, value
    db idx, value, value, idx
    dw idx, value, value, idx
    endmacro

    macro defstringindex idx, string
    db idx, string
    db idx, string
    asciz idx, string
    endmacro

    macro defsprite idx, name
    db 23,27,0,idx
    db 23,27,1
    dw 16,16
    incbin name
    endmacro

    defspace 3, 0xff
    defdbtest 10,11
    defstringindex 2, "Test"

    macro orgspecial start, offset
    org start + offset
    endmacro

load_sprites:
    defsprite 0, "incbin1.bin"
    defsprite 1, "incbin2.bin"

bizar:
    orgspecial 0x1000, 0x10
    ld a,b
