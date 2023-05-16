; local labels before a global label
        ld a,b
@local1:
        ld a,b
        ld hl, @local1
        ld hl, @local2
@local2:
        ld a,b
        ld hl, @local2
        ld hl, @local1
global1:
; local labels between global labels
        ld a,b
@local1:
        ld a,b
        ld hl, @local1
        ld hl, @local2
@local2:
        ld a,b
        ld hl, @local1
        ld hl, @local2
; anonymous labels
@@:     
        ld a,b
        ld hl, @p
        ld hl, @b
        ld hl, @n
        ld hl, @f
global2:
; local labels after last global label
        ld a,b
@local1:
        ld a,b
        ld hl, @local1
        ld hl, @local2
@@:
        ld a,b
        ld hl, @b
        ld hl, @p
@local2:
        ld a,b
        ld hl, @local1
        ld hl, @local2
