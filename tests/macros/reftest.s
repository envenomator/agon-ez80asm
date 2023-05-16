; define macro without arguments
addhla: MACRO
        add a,l
        jr nc, $1
        inc h
$1:
        ld l,a
        ENDMAC addhla

; define marcro with two arguments
pointless: MACRO arg1, arg2
        ld a, arg1
        ld l, arg2
        ENDMAC pointless

; invoke
        addhla
        pointless 10,15
        pointless a,b