#!/bin/bash
# Positive test - assembler needs to pass all tests in all subfolders
# return 0 on succesfull tests (all passed)
# return 1 on issue during test (one or more tests didn't pass correctly)
# return 2 on error in test SETUP 
#

test_number=0
tests_successfull=0

cd tests
rm -f *.bin
for FILE in *; do
    if [ -f "$FILE" ]; then
        test_number=$((test_number+1))
        ../$ASMBIN $FILE -b FF > ../asm.output
        if [ $? -eq 1 ]; then 
            echo "Error in" \'$FILE\'
        else
            tests_successfull=$((tests_successfull+1))
        fi 
    fi
done
rm -f *.bin
cd ..

rm -f asm.output
if [ $test_number -eq $tests_successfull ]; then
    echo "All ($test_number) files assembled succesfully"
    exit 0
else
    exit 1
fi
exit 0
