    ld a,b
    macro test 1,2
    ld a,b
    ld b,c
    endmacro

    macro geinig arg1,arg2,arg3
    ld hl, $40000
    endmacro

    ld a,b

