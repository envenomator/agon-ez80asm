        .assume adl=1
        db 1
        db 2
        .align 16
label1: 
        db 1
        .align 64
label2: 
        db 1
        ld hl, label2
        .align 256
        ld hl, label1
        