@test:
    ld a,b
    jp nz, @test
next:
    ld a,b
@test:
    jp @test
    jp next

    db "String",0,"Next",0
