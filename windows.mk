# Makefile for Windows
OS=WINDOWS
PROJECTNAME=ez80asm.exe

# Tools and arguments
CC=cl.exe
OUTFLAG=/Fo
LFLAGS=/link /Zi /W4
COMMONCFLAGS=/W4 /nologo /DWINDOWS /D_CRT_SECURE_NO_WARNINGS /c $(OUTFLAG)
CFLAGS=/Zi $(COMMONCFLAGS)
RELEASE_LFLAGS=/link /W4 /O2
RELEASE_CFLAGS=/O2 /DNDEBUG $(COMMONCFLAGS)

LINKER=link.exe
LINKERFLAGS=/nologo /subsystem:console /out:
