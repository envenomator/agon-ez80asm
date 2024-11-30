; All values should result in zero

; Testing basic bracket functionality including spacing tolerances in the parser
    ld a, [0]
    ld a, [ 0]
    ld a, [0 ]
    ld a, [ 0 ]
    ld a, [	0	];tab
    ld a, [1-1]
    ld a, [ 1-1 ]
    ld a, [ 1 -1]
    ld a, [ 1 -1 ]
    ld a, [ 1 - 1]
    ld a, [ 1 - 1 ]

    ld a, 5-[5]
    ld a, 5- [5]
    ld a, 5 -[5]
    ld a, 5 - [5]
    ld a, 5 - [ 5]
    ld a, 5 - [5 ]
    ld a, 5 - [ 5 ]

    ld a, -5+[5]
    ld a, -5+ [5]
    ld a, -5 +[5]
    ld a, -5 + [5]
    ld a, -5 + [ 5]
    ld a, -5 + [5 ]
    ld a, -5 + [ 5 ]

    ld a, 42-[2*12]-18
    ld a, 42- [2*12]-18
    ld a, 42 -[2*12]-18
    ld a, 42 - [2*12]-18
    ld a, 42 - [ 2*12]-18
    ld a, 42 - [2*12 ]-18
    ld a, 42 - [ 2*12 ]-18
    ld a, 42 - [ 2 * 12 ]-18
    ld a, 42-[2*12] -18
    ld a, 42-[2*12]- 18
    ld a, 42-[2*12] - 18

; Testing order of operations using brackets
    ld a, [1-1]*64
    ld a, 64*[1-1]
    ld a, 64*[1-0]-64
    ld a, 64*[2/2]-[64/1]

; Testing nested brackets
    ld a, 20-[[[10*2]]]
    ld a, 20-[40-[10*2]]
    ld a, 20-[60-[10*2]-20]
    ld a, 20-[60-[10*2]-20] + 5*[20-[2*10]]
    ld a, [20-[60-[10*2]-20] + 5*[20-[2*10]]]

