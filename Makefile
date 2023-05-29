PROJECTNAME=ez80asm

# Tools and arguments
CC=gcc
LFLAGS=-g -Wall -DUNIX
CFLAGS=$(LFLAGS) -c
OUTFLAG=-o
RELEASE_LFLAGS=-Wall -O2 -DNDEBUG -DUNIX
RELEASE_CFLAGS=$(RELEASE_LFLAGS) -c
.DEFAULT_GOAL := all

# project directories
SRCDIR=src
OBJDIR=obj
BINDIR=bin
# Automatically get all sourcefiles
SRCS=$(wildcard $(SRCDIR)/*.c)
# Automatically get all objects to make from sources
OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
# Target project binary
BIN=$(BINDIR)/$(PROJECTNAME)

# Default rule
all: $(BINDIR) $(OBJDIR) $(BIN)

# Release with optimal settings for release target
release: CFLAGS=$(RELEASE_CFLAGS)
release: LFLAGS=$(RELEASE_LFLAGS)
release: $(BINDIR)
release: $(OBJDIR)
release: $(BIN)

# Linking all compiled objects into final binary
$(BIN):$(OBJS)
ifeq ($(CC),gcc)
	$(CC) $(LFLAGS) $(OBJS) $(OUTFLAG) $@
else
	$(LINKER) $(LINKERFLAGS)$@ $(OBJS)
endif

# Compile each .c file into .o file
$(OBJDIR)/%.o: $(SRCDIR)/%.c
ifeq ($(CC),gcc)
	$(CC) $(CFLAGS) $< $(OUTFLAG) $@
else
	$(CC) $(CFLAGS)$@ $<
endif

$(BINDIR):
	mkdir $(BINDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
ifdef OS
	del /s /q $(BINDIR) >nul 2>&1
	del /s /q $(OBJDIR) >nul 2>&1
else
	$(RM) -r $(BINDIR)/* $(OBJDIR)/*
endif
	rmdir $(BINDIR)
	rmdir $(OBJDIR)