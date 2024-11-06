; expand a macro using arguments in an expression operation
; 1.5 - 1.9 versions should report an error with 'undefined label'
    .macro test arg1, arg2
    ld a, arg1 + 1
    ld a, arg1 + arg2 + 1
    .endmacro

    test 1,2