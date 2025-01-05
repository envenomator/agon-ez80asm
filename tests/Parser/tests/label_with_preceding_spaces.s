; Testing if a label can start with spaces
 label1:
    ld a,0
  label2:
    ld a,1
   label3:
    ld a,2

    @local1:
    @@:
    ld hl, label1
    ld hl, label2
    ld hl, label3
    ld hl, @local1
    ld hl, @p
