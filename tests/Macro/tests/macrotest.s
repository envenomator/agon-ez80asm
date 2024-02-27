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

; define macro with long name and long arguments
        macro abcdefg_bahjelsd__jkdfherkklliix 0123456789abcdef0123456789abcdef, x123456789abcdef0123456789abcdef
        ld a, 0123456789abcdef0123456789abcdef
        ld a, x123456789abcdef0123456789abcdef
        endmacro
; invoke
        addhla ; empty command
        pointless 10,15
        pointless a,b
	    abcdefg_bahjelsd__jkdfherkklliix 10,15

