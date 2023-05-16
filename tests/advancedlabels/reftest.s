; local labels before a global label
        ld a,b
$local1:
        ld a,b
        ld hl, $local1
        ld hl, $local2
$local2:
        ld a,b
        ld hl, $local2
        ld hl, $local1
global1:
SCOPE
; local labels between global labels
        ld a,b
$local1:
        ld a,b
        ld hl, $local1
        ld hl, $local2
$local2:
        ld a,b
        ld hl, $local1
        ld hl, $local2
; anonymous labels
$$:     
        ld a,b
        ld hl, $B
        ld hl, $B
        ld hl, $F
        ld hl, $F
global2:
SCOPE
; local labels after last global label
        ld a,b
$local1:
        ld a,b
        ld hl, $local1
        ld hl, $local2
$$:
        ld a,b
        ld hl, $B
        ld hl, $B
$local2:
        ld a,b
        ld hl, $local1
        ld hl, $local2
