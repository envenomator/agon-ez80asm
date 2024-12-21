; Testing if a labeled argument is correctly replaced using differently sized labels
o: equ 1
ab: equ 2
abc: equ 3
abcd: equ 4

    macro test one ; argument with a length of 3
    ld a, one
    ld a, (one)
    db one
    dw one
    dl one
    dw32 one
    endmacro

    test o
    test ab
    test abc
    test abcd
