#!/bin/bash
# Negative test - assembler needs to fail tests in all subfolders
# return 0 on succesfull test (all failed)
# return 1 on issue during test (one or more tests didn't fail correctly)
# return 2 on error in test SETUP 
#

negtest_number=0
negtest_failed_successfull=0

cd tests
rm -f *.bin
for FILE in *; do
    if [ -f "$FILE" ]; then
        negtest_number=$((negtest_number+1))
        ../$ASMBIN $FILE -b FF > ../asm.output
        if [ $? -eq 0 ]; then 
            echo "Failed to detect error in" \'$FILE\'
        else
            negtest_failed_successfull=$((negtest_failed_successfull+1))
        fi 
    fi
done
rm -f *.bin
cd ..

rm -f asm.output
if [ $negtest_number -eq $negtest_failed_successfull ]; then
    echo "Detected all errors succesfully"
    exit 0
else
    exit 1
fi
exit 0
