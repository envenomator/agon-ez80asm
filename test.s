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
;ld ix, (ix+5)
;ld iy, (ix+5)
;ld ix, (iy+5)
;ld iy, (iy+5)
;ld ix, 0xaabbcc
;ld iy, 0xaabbcc
;ld ix, (0xaabbcc)
;ld iy, (0xaabbcc)
;ld (ix+5),0x10
;ld (iy+5),0x10
;ld (ix+5), b
;ld (iy+5), b
;ld (ix+5),bc
;ld (ix+5),de
;ld (ix+5),hl
;ld (iy+5),bc
;ld (iy+5),de
;ld (iy+5),hl
;ld mb, a
;adl 0
;ld (0xaabbcc),a
;adl 1
;ld (0xaabbcc),a
;adl 1
;ld.sis (0xaabbcc),a
;adl 0
;ld.lil (0xaabbcc),a
adl 1
ld.s (0xaabbcc),ix
ld.s (0xaabbcc),iy