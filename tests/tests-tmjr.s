;add a, (hl)
;adl 1
;add.s a, (hl)
;adl 0
;add.l a, (hl)
;add a, ixh
;add a, ixl
;add a, iyh
;add a, iyl

;ADd A, (HL)
;add a,(ix+5)
;adl 1
;add.s a,(ix+5)
;adl 0
;add.l a,(ix+5)
;add a,(iy+5)
;adl 1
;add.s a,(iy+5)
;adl 0
;add.l a,(iy+5)
;adl 1
;bit.s 0, (HL)
;bit 1, (HL)
;bit 6, (HL)
;bit 7, (HL)
;call nz, 0xffff
;call z, 0xffff
;call nc, 0xffff
;call c, 0xffff
;call po, 0xffff
;call pe, 0xffff
;call p, 0xffff
;call m, 0xffff
;call nc, 0xaabbcc
;adl 0
;call z, $aabbcc
;adl 1
;call z, $aabbcc
;adl 0
;call.is z, $aabbcc
;adl 1
;call.is z, $aabbcc
;adl 0
;call.il z, $aabbcc
;adl 1
;call.il z, $aabbcc
;add a,5
;adl 0
;call $aabbcc
;adl 1
;call $aabbcc
;adl 0
;call.is $aabbcc
;adl 1
;call.is $aabbcc
;adl 0
;call.il $aabbcc
;adl 1
;call.il $aabbcc
;labela:
;    adc a,(hl)
;    add a,5
;labelb:
;    add a,10
;    call labelb
;ccf
;bit 1,(iy+5)
;adc a, ixh
;adc a, ixl
;adc a, iyh
;adc a, iyl

;adc a, (ix+5)
;adl 1
;adc.s a, (ix+5)
;adl 0
;adc.l a, (ix+5)
;
;adc a, (iy+10)
;adl 1
;adc.s a, (iy+10)
;adl 0
;adc.l a, (iy+10) 
;dec ixh
;dec ixl
;dec iyh
;dec iyl
;dec ix
;dec iy

;dec (ix+5)
;dec (iy+5)
;dec a,
;dec b,
;dec l

;dec (hl)
;dec bc
;dec de
;dec hl
;dec sp
;djnz 0x100
;im 0
;im 1
;im 2
;in a, (5)
;in a, (c)
;in b, (c)
;in c, (c)
;in d, (c)
;in e, (c)
;in h, (c)
;in l, (c)
;in0 a, (0x10)
;in0 b, (0x10)
;in0 c, (0x10)
;in0 d, (0x10)
;in0 e, (0x10)
;in0 h, (0x10)
;in0 l, (0x10)
;inc ixh
;inc ixl
;inc iyh
;inc iyl
;inc ix
;inc iy
;inc (hl)
;inc (ix+5)
;inc (iy+5)
;inc a
;inc b
;inc c
;inc d
;inc e
;inc h
;inc l
;inc bc
;inc de
;inc hl
;inc sp
;ind
;ind2
;ind2r
;indm
;indmr
;indr
;indrx
;ini
;ini2
;ini2r
;inim
;inimr
;inir
;inirx
;jp nz, 0xaabbcc
;jp (hl)
;jp (ix)
;jp (iy)
;jp 0xaabbcc
;adl 0
;jp nz, 0xaabbcc
;adl 1
;jp nz, 0xaabbcc
;jp.sis nz, 0xaabbcc
;jp.lil nz, 0xaabbcc
;jr nz, 5
;jr z, 5
;jr nc, 5
;jr c, 5
;jr -5