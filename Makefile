# C Compiler
CC=gcc
# C compiler flags
CFLAGS=-Wall -DCOMMIT=\"$(shell git rev-parse HEAD)\" -DNUMCOMMITS=\"$(shell git rev-list --count HEAD)\"
# Linked compiler flags
LDFLAGS=

# Executable name.
EXE?=fmakegen

RELEASE?=0
ifeq ($(RELEASE),1)
  OBJFOLDER=.release
  CFLAGS+=-DRELEASE -O3
else
  OBJFOLDER=.debug
  CFLAGS+=-DDEBUG -g
endif

# Get source files.
SRC=$(wildcard Src/*.c)
# Get object folder
OBJ=$($(SRC:.c=.o):src=$(OBJFOLDER))
DEP=$(wildcard $(OBJFOLDER)/*.d)

.PHONY: all
all: $(EXE)
$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o$@ $^

# Install and uninstall command
.PHONY: install
install:
# If RELEASE wasn't given just run make with that real quick...
# TODO: Wait does this work... If RELEASE is given then what?... What if user wants debug code to be installed...
ifeq ($(RELEASE),0)

endif
.PHONY: uninstall
	#TODO: Reverse of install...

.PHONY: clean
clean:
	rm -vfr *~ $(EXE)

# Support calls testing
.PHONY: list_objects
list_objects:
	echo $(OBJ)
