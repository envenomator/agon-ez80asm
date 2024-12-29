; Testing only the undocumented Z80 instructions.
.cpu Z80
    inc ixh ; DD 24
    inc ixl ; DD 2C
    inc iyh ; FD 24
    inc iyl ; FD 2C

    sll b ; DDCB 30
    sll c ; DDCB 31 
    sll d ; DDCB 32
    sll e ; DDCB 33
    sll h ; DDCB 34
    sll l ; DDCB 35
    sll a ; DDCB 37

    in (c) ; ED 70
    out (c),0 ; ED 71

    ld ixh,b ; DD 60
    ld ixh,c ; DD 61
    ld ixh,d ; DD 62
    ld ixh,e ; DD 63
    ld ixh,ixh ; DD 64
    ld ixh,ixl ; DD 65
    ld ixh,a ; DD 67

    ld ixl,b ; FD 60
    ld ixl,c ; FD 61
    ld ixl,d ; FD 62
    ld ixl,e ; FD 63
    ld ixl,ixh ; FD 64
    ld ixl,ixl ; FD 65
    ld ixl,a ; FD 67

    ld b,ixh ; 44
    ld b,ixl ; 45
    ld c,ixh ; 4C
    ld c,ixl ; 4D
    ld d,ixh ; 54
    ld d,ixl ; 55
    ld e,ixh ; 5C
    ld e,ixl ; 5D

    ld ixh,0 ; 26
    ld ixl,0 ; 2E

    ld a,ixh ; 7C
    ld a,ixl ; 7D

    inc ixh ; 24
    inc ixl ; 2C

    dec ixh ; 25
    dec ixl ; 2D

    add a,ixh ; 84
    add a,ixl ; 85
    adc a,ixh ; 8C
    adc a,ixl ; 8D

    sub ixh ; 94
    sub ixl ; 95
    sbc a,ixh ; 9C
    sbc a,ixl ; 9D
    and ixh ; A4
    and ixl ; A5
    xor ixh ; AC
    xor ixl ; AD
    or ixh ; B4
    or ixl ; B5
    cp ixh ; BC
    cp ixl ; BD

    inc iyh;24
    dec iyh;25
    ld iyh,0;26
    inc iyl;2C
    dec iyl;2D
    ld iyl,0;2E

    ld b,iyh;44
    ld b,iyl;45
    ld c,iyh;4C
    ld c,iyl;4D
    ld d,iyh;54
    ld d,iyl;55
    ld e,iyh;5C
    ld e,iyl;5D

    ld iyh,b;60
    ld iyh,c;61
    ld iyh,d;62
    ld iyh,e;63
    ld iyh,iyh;64
    ld iyh,iyl;65
    ld iyh,a;67
    ld iyl,b;68
    ld iyl,c;69
    ld iyl,d;6A
    ld iyl,e;6B
    ld iyl,iyh;6C
    ld iyl,iyl;6D
    ld iyl,a;6F

    add a,iyh;84
    add a,iyl;85
    adc a,iyh;8C
    adc a,iyl;8D
    ld a,iyh;7C
    ld a,iyl;7D
    sub iyh;94
    sub iyl;95
    sbc a,iyh;9C
    sbc a,iyl;9D
    and iyh;A4
    and iyl;A5
    xor iyh;AC
    xor iyl;AD
    or iyh;B4
    or iyl;B5
    cp iyh;BC
    cp iyl;BD

    ; DDCB

    rlc (ix+0),b ; DDCB 00
    rlc (ix+0),c ; DDCB 01
    rlc (ix+0),d ; DDCB 02
    rlc (ix+0),e ; DDCB 03
    rlc (ix+0),h ; DDCB 04
    rlc (ix+0),l ; DDCB 05
    rlc (ix+0),a ; DDCB 07

    rrc (ix+0),b ; DDCB 08
    rrc (ix+0),c ; DDCB 09
    rrc (ix+0),d ; DDCB 0A
    rrc (ix+0),e ; DDCB 0B
    rrc (ix+0),h ; DDCB 0C
    rrc (ix+0),l ; DDCB 0D
    rrc (ix+0),a ; DDCB 0F

    rl (ix+0),b ; DDCB 10
    rl (ix+0),c ; DDCB 11
    rl (ix+0),d ; DDCB 12
    rl (ix+0),e ; DDCB 13
    rl (ix+0),h ; DDCB 14
    rl (ix+0),l ; DDCB 15
    rl (ix+0),a ; DDCB 17

    rr (ix+0),b ; DDCB 18
    rr (ix+0),c ; DDCB 19
    rr (ix+0),d ; DDCB 1A
    rr (ix+0),e ; DDCB 1B
    rr (ix+0),h ; DDCB 1C
    rr (ix+0),l ; DDCB 1D
    rr (ix+0),a ; DDCB 1F

    sla (ix+0),b ; DDCB 20
    sla (ix+0),c ; DDCB 21
    sla (ix+0),d ; DDCB 22
    sla (ix+0),e ; DDCB 23
    sla (ix+0),h ; DDCB 24
    sla (ix+0),l ; DDCB 25
    sla (ix+0),a ; DDCB 27

    sra (ix+0),b ; DDCB 28
    sra (ix+0),c ; DDCB 29
    sra (ix+0),d ; DDCB 2A
    sra (ix+0),e ; DDCB 2B
    sra (ix+0),h ; DDCB 2C
    sra (ix+0),l ; DDCB 2D
    sra (ix+0),a ; DDCB 2F

    sll (ix+0),b ; DDCB 30
    sll (ix+0),c ; DDCB 31
    sll (ix+0),d ; DDCB 32
    sll (ix+0),e ; DDCB 33
    sll (ix+0),h ; DDCB 34
    sll (ix+0),l ; DDCB 35
    sll (ix+0)   ; DDCB 36
    sll (ix+0),a ; DDCB 37

    srl (ix+0),b ; DDCB 38
    srl (ix+0),c ; DDCB 39
    srl (ix+0),d ; DDCB 3A
    srl (ix+0),e ; DDCB 3B
    srl (ix+0),h ; DDCB 3C
    srl (ix+0),l ; DDCB 3D
    srl (ix+0),a ; DDCB 3F

    res 0,(ix+0),b ; DDCB 80
    res 0,(ix+0),c ; DDCB 81
    res 0,(ix+0),d ; DDCB 82
    res 0,(ix+0),e ; DDCB 83
    res 0,(ix+0),h ; DDCB 84
    res 0,(ix+0),l ; DDCB 85
    res 0,(ix+0),a ; DDCB 87
    
    res 1,(ix+0),b ; DDCB 88
    res 1,(ix+0),c ; DDCB 89
    res 1,(ix+0),d ; DDCB 8A
    res 1,(ix+0),e ; DDCB 8B
    res 1,(ix+0),h ; DDCB 8C
    res 1,(ix+0),l ; DDCB 8D
    res 1,(ix+0),a ; DDCB 8F
    
    res 2,(ix+0),b ; DDCB 90
    res 2,(ix+0),c ; DDCB 91
    res 2,(ix+0),d ; DDCB 92
    res 2,(ix+0),e ; DDCB 93
    res 2,(ix+0),h ; DDCB 94
    res 2,(ix+0),l ; DDCB 95
    res 2,(ix+0),a ; DDCB 97
    
    res 3,(ix+0),b ; DDCB 98
    res 3,(ix+0),c ; DDCB 99
    res 3,(ix+0),d ; DDCB 9A
    res 3,(ix+0),e ; DDCB 9B
    res 3,(ix+0),h ; DDCB 9C
    res 3,(ix+0),l ; DDCB 9D
    res 3,(ix+0),a ; DDCB 9F
    
    res 4,(ix+0),b ; DDCB A0
    res 4,(ix+0),c ; DDCB A1
    res 4,(ix+0),d ; DDCB A2
    res 4,(ix+0),e ; DDCB A3
    res 4,(ix+0),h ; DDCB A4
    res 4,(ix+0),l ; DDCB A5
    res 4,(ix+0),a ; DDCB A7
    
    res 5,(ix+0),b ; DDCB A8
    res 5,(ix+0),c ; DDCB A9
    res 5,(ix+0),d ; DDCB AA
    res 5,(ix+0),e ; DDCB AB
    res 5,(ix+0),h ; DDCB AC
    res 5,(ix+0),l ; DDCB AD
    res 5,(ix+0),a ; DDCB AF
    
    res 6,(ix+0),b ; DDCB B0
    res 6,(ix+0),c ; DDCB B1
    res 6,(ix+0),d ; DDCB B2
    res 6,(ix+0),e ; DDCB B3
    res 6,(ix+0),h ; DDCB B4
    res 6,(ix+0),l ; DDCB B5
    res 6,(ix+0),a ; DDCB B7
    
    res 7,(ix+0),b ; DDCB B8
    res 7,(ix+0),c ; DDCB B9
    res 7,(ix+0),d ; DDCB BA
    res 7,(ix+0),e ; DDCB BB
    res 7,(ix+0),h ; DDCB BC
    res 7,(ix+0),l ; DDCB BD
    res 7,(ix+0),a ; DDCB B7
    
    set 0,(ix+0),b ; DDCB C0
    set 0,(ix+0),c ; DDCB C1
    set 0,(ix+0),d ; DDCB C2
    set 0,(ix+0),e ; DDCB C3
    set 0,(ix+0),h ; DDCB C4
    set 0,(ix+0),l ; DDCB C5
    set 0,(ix+0),a ; DDCB C7
    
    set 1,(ix+0),b ; DDCB C8
    set 1,(ix+0),c ; DDCB C9
    set 1,(ix+0),d ; DDCB CA
    set 1,(ix+0),e ; DDCB CB
    set 1,(ix+0),h ; DDCB CC
    set 1,(ix+0),l ; DDCB CD
    set 1,(ix+0),a ; DDCB CF
    
    set 2,(ix+0),b ; DDCB D0
    set 2,(ix+0),c ; DDCB D1
    set 2,(ix+0),d ; DDCB D2
    set 2,(ix+0),e ; DDCB D3
    set 2,(ix+0),h ; DDCB D4
    set 2,(ix+0),l ; DDCB D5
    set 2,(ix+0),a ; DDCB D7
    
    set 3,(ix+0),b ; DDCB D8
    set 3,(ix+0),c ; DDCB D9
    set 3,(ix+0),d ; DDCB DA
    set 3,(ix+0),e ; DDCB DB
    set 3,(ix+0),h ; DDCB DC
    set 3,(ix+0),l ; DDCB DD
    set 3,(ix+0),a ; DDCB DF
    
    set 4,(ix+0),b ; DDCB E0
    set 4,(ix+0),c ; DDCB E1
    set 4,(ix+0),d ; DDCB E2
    set 4,(ix+0),e ; DDCB E3
    set 4,(ix+0),h ; DDCB E4
    set 4,(ix+0),l ; DDCB E5
    set 4,(ix+0),a ; DDCB E7
    
    set 5,(ix+0),b ; DDCB E8
    set 5,(ix+0),c ; DDCB E9
    set 5,(ix+0),d ; DDCB EA
    set 5,(ix+0),e ; DDCB EB
    set 5,(ix+0),h ; DDCB EC
    set 5,(ix+0),l ; DDCB ED
    set 5,(ix+0),a ; DDCB EF
    
    set 6,(ix+0),b ; DDCB F0
    set 6,(ix+0),c ; DDCB F1
    set 6,(ix+0),d ; DDCB F2
    set 6,(ix+0),e ; DDCB F3
    set 6,(ix+0),h ; DDCB F4
    set 6,(ix+0),l ; DDCB F5
    set 6,(ix+0),a ; DDCB F7
    
    set 7,(ix+0),b ; DDCB F8
    set 7,(ix+0),c ; DDCB F9
    set 7,(ix+0),d ; DDCB FA
    set 7,(ix+0),e ; DDCB FB
    set 7,(ix+0),h ; DDCB FC
    set 7,(ix+0),l ; DDCB FD
    set 7,(ix+0),a ; DDCB FF
    
    ; FDCB
    rlc (iy+0),b ; FDCB 00
    rlc (iy+0),c ; FDCB 01
    rlc (iy+0),d ; FDCB 02
    rlc (iy+0),e ; FDCB 03
    rlc (iy+0),h ; FDCB 04
    rlc (iy+0),l ; FDCB 05
    rlc (iy+0),a ; FDCB 07

    rrc (iy+0),b ; FDCB 08
    rrc (iy+0),c ; FDCB 09
    rrc (iy+0),d ; FDCB 0A
    rrc (iy+0),e ; FDCB 0B
    rrc (iy+0),h ; FDCB 0C
    rrc (iy+0),l ; FDCB 0D
    rrc (iy+0),a ; FDCB 0F

    rl (iy+0),b ; FDCB 10
    rl (iy+0),c ; FDCB 11
    rl (iy+0),d ; FDCB 12
    rl (iy+0),e ; FDCB 13
    rl (iy+0),h ; FDCB 14
    rl (iy+0),l ; FDCB 15
    rl (iy+0),a ; FDCB 17

    rr (iy+0),b ; FDCB 18
    rr (iy+0),c ; FDCB 19
    rr (iy+0),d ; FDCB 1A
    rr (iy+0),e ; FDCB 1B
    rr (iy+0),h ; FDCB 1C
    rr (iy+0),l ; FDCB 1D
    rr (iy+0),a ; FDCB 1F

    sla (iy+0),b ; FDCB 20
    sla (iy+0),c ; FDCB 21
    sla (iy+0),d ; FDCB 22
    sla (iy+0),e ; FDCB 23
    sla (iy+0),h ; FDCB 24
    sla (iy+0),l ; FDCB 25
    sla (iy+0),a ; FDCB 27

    sra (iy+0),b ; FDCB 28
    sra (iy+0),c ; FDCB 29
    sra (iy+0),d ; FDCB 2A
    sra (iy+0),e ; FDCB 2B
    sra (iy+0),h ; FDCB 2C
    sra (iy+0),l ; FDCB 2D
    sra (iy+0),a ; FDCB 2F

    sll (iy+0),b ; FDCB 30
    sll (iy+0),c ; FDCB 31
    sll (iy+0),d ; FDCB 32
    sll (iy+0),e ; FDCB 33
    sll (iy+0),h ; FDCB 34
    sll (iy+0),l ; FDCB 35
    sll (iy+0)   ; FDCB 36
    sll (iy+0),a ; FDCB 37

    srl (iy+0),b ; FDCB 38
    srl (iy+0),c ; FDCB 39
    srl (iy+0),d ; FDCB 3A
    srl (iy+0),e ; FDCB 3B
    srl (iy+0),h ; FDCB 3C
    srl (iy+0),l ; FDCB 3D
    srl (iy+0),a ; FDCB 3F

    res 0,(iy+0),b ; FDCB 80
    res 0,(iy+0),c ; FDCB 81
    res 0,(iy+0),d ; FDCB 82
    res 0,(iy+0),e ; FDCB 83
    res 0,(iy+0),h ; FDCB 84
    res 0,(iy+0),l ; FDCB 85
    res 0,(iy+0),a ; FDCB 87
    
    res 1,(iy+0),b ; FDCB 88
    res 1,(iy+0),c ; FDCB 89
    res 1,(iy+0),d ; FDCB 8A
    res 1,(iy+0),e ; FDCB 8B
    res 1,(iy+0),h ; FDCB 8C
    res 1,(iy+0),l ; FDCB 8D
    res 1,(iy+0),a ; FDCB 8F
    
    res 2,(iy+0),b ; FDCB 90
    res 2,(iy+0),c ; FDCB 91
    res 2,(iy+0),d ; FDCB 92
    res 2,(iy+0),e ; FDCB 93
    res 2,(iy+0),h ; FDCB 94
    res 2,(iy+0),l ; FDCB 95
    res 2,(iy+0),a ; FDCB 97
    
    res 3,(iy+0),b ; FDCB 98
    res 3,(iy+0),c ; FDCB 99
    res 3,(iy+0),d ; FDCB 9A
    res 3,(iy+0),e ; FDCB 9B
    res 3,(iy+0),h ; FDCB 9C
    res 3,(iy+0),l ; FDCB 9D
    res 3,(iy+0),a ; FDCB 9F
    
    res 4,(iy+0),b ; FDCB A0
    res 4,(iy+0),c ; FDCB A1
    res 4,(iy+0),d ; FDCB A2
    res 4,(iy+0),e ; FDCB A3
    res 4,(iy+0),h ; FDCB A4
    res 4,(iy+0),l ; FDCB A5
    res 4,(iy+0),a ; FDCB A7
    
    res 5,(iy+0),b ; FDCB A8
    res 5,(iy+0),c ; FDCB A9
    res 5,(iy+0),d ; FDCB AA
    res 5,(iy+0),e ; FDCB AB
    res 5,(iy+0),h ; FDCB AC
    res 5,(iy+0),l ; FDCB AD
    res 5,(iy+0),a ; FDCB AF
    
    res 6,(iy+0),b ; FDCB B0
    res 6,(iy+0),c ; FDCB B1
    res 6,(iy+0),d ; FDCB B2
    res 6,(iy+0),e ; FDCB B3
    res 6,(iy+0),h ; FDCB B4
    res 6,(iy+0),l ; FDCB B5
    res 6,(iy+0),a ; FDCB B7
    
    res 7,(iy+0),b ; FDCB B8
    res 7,(iy+0),c ; FDCB B9
    res 7,(iy+0),d ; FDCB BA
    res 7,(iy+0),e ; FDCB BB
    res 7,(iy+0),h ; FDCB BC
    res 7,(iy+0),l ; FDCB BD
    res 7,(iy+0),a ; FDCB B7
    
    set 0,(iy+0),b ; FDCB C0
    set 0,(iy+0),c ; FDCB C1
    set 0,(iy+0),d ; FDCB C2
    set 0,(iy+0),e ; FDCB C3
    set 0,(iy+0),h ; FDCB C4
    set 0,(iy+0),l ; FDCB C5
    set 0,(iy+0),a ; FDCB C7
    
    set 1,(iy+0),b ; FDCB C8
    set 1,(iy+0),c ; FDCB C9
    set 1,(iy+0),d ; FDCB CA
    set 1,(iy+0),e ; FDCB CB
    set 1,(iy+0),h ; FDCB CC
    set 1,(iy+0),l ; FDCB CD
    set 1,(iy+0),a ; FDCB CF
    
    set 2,(iy+0),b ; FDCB D0
    set 2,(iy+0),c ; FDCB D1
    set 2,(iy+0),d ; FDCB D2
    set 2,(iy+0),e ; FDCB D3
    set 2,(iy+0),h ; FDCB D4
    set 2,(iy+0),l ; FDCB D5
    set 2,(iy+0),a ; FDCB D7
    
    set 3,(iy+0),b ; FDCB D8
    set 3,(iy+0),c ; FDCB D9
    set 3,(iy+0),d ; FDCB DA
    set 3,(iy+0),e ; FDCB DB
    set 3,(iy+0),h ; FDCB DC
    set 3,(iy+0),l ; FDCB DD
    set 3,(iy+0),a ; FDCB DF
    
    set 4,(iy+0),b ; FDCB E0
    set 4,(iy+0),c ; FDCB E1
    set 4,(iy+0),d ; FDCB E2
    set 4,(iy+0),e ; FDCB E3
    set 4,(iy+0),h ; FDCB E4
    set 4,(iy+0),l ; FDCB E5
    set 4,(iy+0),a ; FDCB E7
    
    set 5,(iy+0),b ; FDCB E8
    set 5,(iy+0),c ; FDCB E9
    set 5,(iy+0),d ; FDCB EA
    set 5,(iy+0),e ; FDCB EB
    set 5,(iy+0),h ; FDCB EC
    set 5,(iy+0),l ; FDCB ED
    set 5,(iy+0),a ; FDCB EF
    
    set 6,(iy+0),b ; FDCB F0
    set 6,(iy+0),c ; FDCB F1
    set 6,(iy+0),d ; FDCB F2
    set 6,(iy+0),e ; FDCB F3
    set 6,(iy+0),h ; FDCB F4
    set 6,(iy+0),l ; FDCB F5
    set 6,(iy+0),a ; FDCB F7
    
    set 7,(iy+0),b ; FDCB F8
    set 7,(iy+0),c ; FDCB F9
    set 7,(iy+0),d ; FDCB FA
    set 7,(iy+0),e ; FDCB FB
    set 7,(iy+0),h ; FDCB FC
    set 7,(iy+0),l ; FDCB FD
    set 7,(iy+0),a ; FDCB FF
