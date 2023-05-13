test: equ 't'
test2:    equ 'b'
l:    equ '+'
    cp 'a' + 0
    cp '+' + 0
    cp '+' + 1
    cp '+'
    cp ','
    db 0,0,0,'a','b',',','+','\''
    db '\''+1
    db '/'
    db '/'+'/'
    db "ab\"'"
    db ','
    blkb 15,'+' + 0 ;test
    ld a, (ix+'0')
    ld a, test
    ld a, test2
