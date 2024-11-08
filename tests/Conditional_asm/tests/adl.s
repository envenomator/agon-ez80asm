; Test conditional ADL
;

    assume ADL=1; disable startup assumptions and set up for failure if not implemented

    .if 1
    assume ADL=0
    .else       ; will run if not implemented, resulting in ADL mode 1, resulting in an error later on
    assume ADL=1
    .endif

    ld hl, 0 ; this should produce 0x21 0x00 0x00, if properly implemented
