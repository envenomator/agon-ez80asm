labela: .equ 2
labelb:
; basic operations
    ld de, 2+1
    ld de, 2-1
    ld de, 2*2
    ld de, 4<<2
    ld de, 4>>2
    ld de, -4
    ld de, 8&1
    ld de, 8|1
    ld de, 12^25
    ld de, ~0
    ld de, 8/2
; basic operations with spaces
    ld de, 2 +1
    ld de, 2 -1
    ld de, 2 *2
    ld de, 4 <<2
    ld de, 4 >>2
    ld de, - 4
    ld de, 8 &1
    ld de, 8 |1
    ld de, 12 ^25
    ld de, ~ 0
    ld de, 8 /2

    ld de, 2+ 1
    ld de, 2- 1
    ld de, 2* 2
    ld de, 4<< 2
    ld de, 4>> 2
    ld de, - 4
    ld de, 8& 1
    ld de, 8| 1
    ld de, 12^ 25
    ld de, ~ 0
    ld de, 8/ 2

    ld de, 2 + 1
    ld de, 2 - 1
    ld de, 2 * 2
    ld de, 4 << 2
    ld de, 4 >> 2
    ld de, 8 & 1
    ld de, 8 | 1
    ld de, 12 ^ 25
    ld de, ~ 0
    ld de, 8 / 2

; unsary operations
rows:   equ 30
cols:   equ 10

    ld de, -10
    ld de, -cols
    ld de, -30*-10
    ld de, -rows*-cols
    ld de, 30++10
    ld de, rows++cols
    ld de, 30+-10
    ld de, rows+-cols
    ld de, 30-+10
    ld de, rows-+cols
    ld de, 30--10
    ld de, rows--cols
    ld de, 30+~0
    ld de, rows+~0
    ld de, 30+~ffh
    ld de, rows+~ffh
    ld de, -30-~0
    ld de, -rows-~0

; basic operations with spaces
    ld de, 2	+1
    ld de, 2	-1
    ld de, 2	*2
    ld de, 4	<<2
    ld de, 4	>>2
    ld de, -	4
    ld de, 8    &1
    ld de, 8    |1
    ld de, 12   ^25
    ld de, ~    0
    ld de, 8    /2

    ld de, 2+	1
    ld de, 2-	1
    ld de, 2*	2
    ld de, 4<<	2
    ld de, 4>>	2
    ld de, -	4
    ld de, 8&   1
    ld de, 8|   1
    ld de, 12^  25
    ld de, ~    0
    ld de, 8/   2

    ld de, 2	+	1
    ld de, 2	-	1
    ld de, 2	*	2
    ld de, 4	<<	2
    ld de, 4	>>	2
    ld de, 8    &   1
    ld de, 8    |   1
    ld de, 12   ^   25
    ld de, ~    0
    ld de, 8    /   2

; compound operations
    ld de, 2+2+2
    ld de, 2 + 2 + 2
    ld de, 8 - 4 + 2

; label operations
    ld hl, labelb + 2
    ld hl, labelb - 2
    ld hl, labelb * 2
    ld hl, labelb << 2
    ld hl, labelb >> 2

    ld hl, labelb + labela
    ld hl, labelb - labela
    ld hl, labelb * labela
    ld hl, labelb << labela
    ld hl, labelb >> labela

    ld hl, labelb + labela + 1
    ld hl, labelb - labela + 1
    ld hl, labelb * labela + 1
    ld hl, labelb << labela + 1
    ld hl, labelb >> labela + 1

