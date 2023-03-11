    ld a,b
    macro test 1,2
    ld a,b
    ld b,c
    endmacro

    macro geinig arg1,arg2,arg3
    ld hl, $40000
    ld a, arg1
    endmacro

    ld a,b
    geinig a,0,0
    geinig b,0,0
    geinig x,0,0

