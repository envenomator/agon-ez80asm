ifeq ($(OS),Windows_NT)
	include Windows.mk
else
	include Unix.mk
endif

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
all:setupdirs $(BIN)

# Release with optimal settings for release target
release: CFLAGS=$(RELEASE_CFLAGS)
release: LFLAGS=$(RELEASE_LFLAGS)
release: clean
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

# Initial project setup helper
setupdirs:
	@echo Creating project directories
	@if test -d $(SRCDIR); then echo Re-using existing directory \'$(SRCDIR)\' ; else mkdir $(SRCDIR); fi
	@if test -d $(OBJDIR); then echo Re-using existing directory \'$(OBJDIR)\' ; else mkdir $(OBJDIR); fi
	@if test -d $(BINDIR); then echo Re-using existing directory \'$(BINDIR)\' ; else mkdir $(BINDIR); fi

clean:
	$(RM) -r $(BINDIR)/* $(OBJDIR)/*
