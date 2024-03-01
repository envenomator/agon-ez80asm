    blkb 2,128+127-255+4/2*2<<1>>1&0x04|0x04;should be 4
    blkb 2,-128;should be 0x80
    blkb 2, 0x55^0;should be 0x55
    blkb 2, ~0;should be 0xff
    blkb 2, 0xff-0xff+1;should be 1
    blkw 2, 0xffff/2-0x7000+1*2<<1>>1;should be 0x2000
    blkw 2, 0x5555^0;should be 0x5555
    blkw 2, -32768;should be 0x8000
    blkw 2, ~0;should be 0xffff
    blkp 2, 0xffffff-0xffff00+1/2*2<<1>>1;should be 0x100
    blkp 2, 0x555555^0;should be 0x555555
    blkp 2, -8388608;should be 0x800000
    blkp 2, 0xffffff-0xffffff+1;should be 1
    blkl 2, 0xffffffff-0xffffffff+0x100<<1>>1/2*2;should be 0x100
    blkl 2, 0x55555555^0;should be 0x55555555
    blkl 2, ~0;should be 0xffffffff
    blkl 2, -2147483648;should be 0x80000000