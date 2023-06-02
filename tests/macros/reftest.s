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

; define macro with long name and long arguments
macrolong: MACRO arg1, arg2
        ld a, arg1
        ld a, arg2
        ENDMAC macrolong

; invoke
        addhla
        pointless 10,15
        pointless a,b
        macrolong 10,15
