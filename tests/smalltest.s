.org 0x40000
labela:
    add a, b
labelb:
    add a, (hl)
    add a, 0x10
    add a, b
    jp labelb
