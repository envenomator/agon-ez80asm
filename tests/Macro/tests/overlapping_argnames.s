; Test for overlapping argument names
; This should work from 1.10+
    macro test arg, argument
    ld a, arg + argument
    endmacro

    test 1,2