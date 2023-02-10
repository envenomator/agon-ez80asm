;adl 1
;adc.s a, (hl)
;adl 0
;adc.l a, (hl)
;adc a, (hl)
;adl 1
;adc.s a, (ix+0x10)
;adc.s a, (iy+0x10)
;adl 0
;adc.l a, (ix+0x05)
;adc.l a, (iy+0x05)
;adc hl, bc
;adc hl, de
;adc hl, hl
;adc hl, sp
;adc a,0xff
;adc a,0xff
;adc a,b
;adc a,c
;adc a,d
;adc a,e
;adc a,h
;adc a,l
;adc a,a
;adc a, ixh
;adc a, ixl
;adc a, iyh
;adc a, iyl
;adc a, (ix+5)
;adc a, (iy+5)
;
;adl 1
;adc.s hl, bc
;adc.s hl, de
;adc.s hl, hl
;adc.s hl, sp
;adl 0
;adc.l hl, bc
;adc.l hl, de
;adc.l hl, hl
;adc.l hl, sp
