@echo off
set targetdir=releases
echo Packaging Agon binaries
if not exist "%targetdir%" mkdir %targetdir%
echo Converting iHex files to binary
objcopy -I ihex src\Debug\ez80asm.hex -O binary %targetdir%\ez80asm.ldr
objcopy -I ihex src\moscmd\Release\ez80asm.hex -O binary %targetdir%\ez80asm.bin
cd %targetdir%
echo Zipping binary files
tar -a -c -f ez80asm_agon.zip ez80asm.*
echo Cleanup
del ez80asm.bin
del ez80asm.ldr
@echo Done