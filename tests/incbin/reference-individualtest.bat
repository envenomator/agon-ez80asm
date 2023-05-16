@echo off
if exist test.bin (
    copy test.bin reference.bin 
) else (
    echo Please assemble test.s to test.bin first
)
