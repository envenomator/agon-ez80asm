before: equ 0x01

     db before, after1
     dw before, after1
tst: equ before
     assume adl=before
    blkb before,after1
    align before
    fillbyte before

after1: equ 0x01
after2: equ 0x02