#!/bin/bash
# Negative test - assembler needs to fail this test
# return 0 on succesfull test
# return 1 on issue during test
# return 2 on error in test SETUP
#
if [ -f "test.s" ]; then
    $ASMBIN test.s -b FF > asm.output
    if [ $? -eq 0 ]; then 
        echo "Assembler incorrectly accepting negative test"
        exit 1
    else
        echo "Assembler correctly failing test input"
    fi
else
    echo "test.s not present"
    exit 2
fi
exit 0
