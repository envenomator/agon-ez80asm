; invoke a macro on the same line as a label
; 1.8+ versions should report an error with 'undefined label'
    .macro test
    ld a,b
    .endmacro

label: test
    jp label