    macro test
    ld a,b
    ld b,c
    ld c,d
    endmacro

    macro test2
    ld a,b
    endmacro

    test
    test2
    ld a,b
