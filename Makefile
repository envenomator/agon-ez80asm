PROJECTNAME=project
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
# Tools and arguments
CC=gcc
CFLAGS=-g -Wall

# Default rule
all:$(BIN)

# Release with optimal settings for release target
release: CFLAGS=-Wall -O2 -DNDEBUG
release: clean
release: $(BIN)

# Linking all compiled objects into final binary
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Compile each .c file into .o file
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Initial project setup helper
setupdirs:
	@echo Creating project directories
	@if test -d $(SRCDIR); then echo Re-using existing directory \'$(SRCDIR)\' ; else mkdir $(SRCDIR); fi
	@if test -d $(OBJDIR); then echo Re-using existing directory \'$(OBJDIR)\' ; else mkdir $(OBJDIR); fi
	@if test -d $(BINDIR); then echo Re-using existing directory \'$(BINDIR)\' ; else mkdir $(BINDIR); fi

clean:
	$(RM) -r $(BINDIR)/* $(OBJDIR)/*
