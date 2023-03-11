    ld a,b
    macro sum arg1, arg2
    ld a,arg1
    ld b,arg2
    add a,b
    endmacro

    macro geinig arg1,arg2,arg3
    ld hl, $40000
    ld a, arg1
    endmacro

    geinig a,0,0
    sum 10,15
