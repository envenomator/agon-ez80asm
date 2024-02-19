; we'll use the directive without ., or the reference ZDS assembler barks at us
; byte tests
        db 5
        db 1,2,3
        db 10;comment
        byte 5
        byte 1,2,3
        byte 10;comment
        ascii 5
        ascii 1,2,3
        ascii 10;comment
        DB 5
        DB 1,2,3
        DB 10;comment
label:
        jp label2
; 16-bit word tests
        dw 5
        dw 1,2,3
        dw 10;comment
        dw ffffh
        dw label
; 24-bit word tests
        DW24 5
        DW24 1,2,3
        DW24 10;comment
        DW24 ffffh
        DW24 label
        dw24 5
        dw24 1,2,3
        dw24 10;comment
        dw24 ffffh
        dw24 label
; define space
        blkb 10,0
        blkb 10,0;uninitialized. Will be different between assemblers
        blkb 10,FFh
; define blocks
        blkb 16, FFh
        blkb 10h, FFh
        blkb 16,0;uninitialized. Will be different between assemblers
        blkw 16, AAFFh
        blkw 10h, AAFFh
        blkw 16,0;uninitialized. Will be different between assemblers
        blkp 16, AABBFFh
        blkp 10h, AABBFFh
        blkp 10h, label
        blkp 16,0;uninitialized. Will be different between assemblers
        blkl 16, AABBCCFFh
        blkl 10h, AABBCCFFh
        blkl 16,0;uninitialized. Will be different between assemblers
label2:
        jp label