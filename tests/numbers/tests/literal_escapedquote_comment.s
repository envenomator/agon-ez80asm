; Testing the getOperandToken parser
    ld a, '\''+1;Errors out in v1.11
    ld a, '\''+1 ;Same
    db '\''+1;Errors out in v1.11
    db '\''+1 ;Same

