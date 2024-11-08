; Test conditional incbin

    ; Test basics
    .if 1
    .incbin "incbin.inc"
    .else
    .db 0
    .endif
   
    ; Test avoidance of including
    .if 0
    .incbin "incbin.inc"
    .else
    .db 0
    .endif
