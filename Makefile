#PROJECTNAME=project
ifeq ($(OS),Windows_NT)
	include windows.mk
else
	include unix.mk
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
all: setupdirs $(BIN)

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
	@if ! test -d $(SRCDIR); then mkdir $(SRCDIR); fi
	@if ! test -d $(OBJDIR); then mkdir $(OBJDIR); fi
	@if ! test -d $(BINDIR); then mkdir $(BINDIR); fi

clean:
	$(RM) -r $(BINDIR)/* $(OBJDIR)/*
