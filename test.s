labela:
	adl 1
	adl 0
	;ld.lil (0x102030), bc
	;ld.lil (0x102030), de
	;ld.lil (0x102030), hl
	;ld.lil (0x102030), sp
	;ld.sis (0x102030), iy
	;ld.sis (0x102030), ix
	;ld.is (0x102030),a <= issue
	;ld (ix+4), ix
	;ld (ix+5), iy
	;ld (iy+4), ix
	;ld (iy+5), iy
	;ld (ix+4), bc
	;ld.s (ix+5), de
	;ld.s (ix+4), a
	;ld (ix+5), b
	;ld.s (ix+4),5
	;ld.s (iy+4),5

	;ld.sis ix, 0x102030
	;ld a, (hl)
	;ld ix, (0x102030)
	;ld de, 0xaabbcc
	;ld bc, (0xaabbcc)
	;ld a, (0xaabb)
	;ld iy, (ix+$2);
	;ld iy, (iy+$2);
	;ld ix, (hl)
	;ld iy, (hl)
	;ld ixh, 0xff
	;ld ixl, 0xff
	;ld iyh, 0xff
	;ld iyl, 0xff
	;ld i, a
	;ld i, hl
	;ld iyh, iyh
	;ld iyh, iyl
	;ld iyl, iyh
	;ld iyl, iyl
	;ld ixh, ixh
	;ld ixh, ixl
	;ld ixl, ixh
	;ld ixl, ixl
	;ld ixh, a
	;ld ixl, a
	;ld iyh, a
	;ld iyl, a
	;ld mb, a
	;ld r, a
	;ld (hl), 10101010b
	;ld (hl), ix
	;ld (hl), iy
	;ld (hl),b
	;ld (hl),c
	;ld (hl),d
	;ld (hl),e
	;ld (hl),h
	;ld (hl),l
	;ld (hl),a
	;ld (bc),a
	;ld (de),a
	;ld (hl),a
	;ld hl, 0x0400ff
	;ld.sis a,(0x0400ff)
	;ld.s sp,ix
	;ld.s sp,iy
	;ld hl,i
	;ld.s sp,hl
	;ld.s bc,(ix+2)
	;ld.l bc,(hl)
	;ld a, (0x040000)
	;ld a, (de)
	;ld a,(ix+2)
	;ld a, 0b11111111
	;ld a,$40
	;ld b,$40
	;ld c,40h
	;ld a,0x10
	;ld a,10h
	;ld a,b
	;ld c,c
	;ld d,d
	;ld a,e
	;ld a,h
	;ld a,l
	;ld a,a
	;ld a,ixh
labelb:
	;ld a,ixl
	;ld a,iyh
	;ld a,iyl
	;ld b,ixh
	;ld b,ixl
	;ld b,iyh
	;ld b,iyl
	;ld c,ixh
	;ld c,ixl
	;ld c,iyh
	;ld c,iyl
	;ld d,ixh
	;ld d,ixl
	;ld d,iyh
	;ld d,iyl
	;ld e,ixh
	;ld e,ixl
	;ld e,iyh
	;ld e,iyl
