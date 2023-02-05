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
bit 1,(ix+5)