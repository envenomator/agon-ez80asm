; define macro without arguments
        macro addhla
        add a,l
        jr nc, @1
        inc h
@1:
        ld l,a
        endmacro

; define marcro with two arguments
        macro pointless arg1, arg2
        ld a, arg1
        ld l, arg2
        endmacro

; invoke
        addhla ; empty command
        pointless 10,15
        pointless a,b
