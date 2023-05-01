@test:
    ld a,b
    jp nz, @test
next:
@test:
    jp @test
