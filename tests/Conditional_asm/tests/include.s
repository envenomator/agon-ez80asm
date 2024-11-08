; Test conditional includes and persistence of if/else/endif in these included files

    .if 1
    .include "include.inc"
    .else
    .db 0
    .endif
