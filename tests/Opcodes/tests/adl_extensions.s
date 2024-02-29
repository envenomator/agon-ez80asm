    assume adl=0
    ld a,(aabbh)
    ld.lil a,(aabbh)

    assume adl=1
    ld a,(aabbh)
    ld.sis a,(aabbh)

    assume adl=0
    jp (hl)
    jp.l (hl)
    jp.lis (hl)

    assume adl=1
    jp (hl)
    jp.s (hl)
    jp.sil (hl)