; Testing illegal argument name (number)

    macro test $1
    ld a, $1
    endmacro

    test 0
