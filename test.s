;ld a,i
;ld a, (ix+5)
;ld a, (iy+5)
;ld a, mb
;adl 0
;ld.lil a, (0x102030)
;ld a,r
;ld.s a, (bc)
;ld.s a, (de)
;ld.s a, (hl)
;ld hl,i
;ld (hl),ix
;ld (hl),iy
;ld (hl), 0x50
;ld (hl), A
;ld (hl), b
;ld (hl), bc
;ld (hl), de
;ld (hl), hl
;ld i, hl
;ld i, a
;ld ixh, 5
;ld ixl, 5
;ld iyh, 5
;ld iyl, 5
;ld ixh, a
;ld ixl, a
;ld iyh, a
;ld iyl, a
;ld ixh, ixh
;ld ixh, ixl
;ld ixl, ixh
;ld ixl, ixl
;ld iyh, iyh
;ld iyh, iyl
;ld iyl, iyh
;ld iyl, iyl
;ld ix,(hl)
;ld iy,(hl)
ld ix, (ix+5)
ld iy, (ix+5)
ld ix, (iy+5)
ld iy, (iy+5)