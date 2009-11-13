# Universal targets for Plexil Makefiles.  This is not a standalone make file,
# and must be included to complete a full make file.

# This file assumes the following macros are properly defined prior to
# this file's inclusion.  Some are the programmer's responsibility, and some
# are found in "standard-defs.make"
#
#       PRELINKER - command to invoke C++ prelinker
#       INC - .H files
#       SRC - .C files
#       TAG - the command to create an Emacs tags table
#       SVN_FILES - a list of all files under SVN control
#       INCLUDES - "-I" flags
#       RM - command to remove files
#       DEPEND - command to generate header dependencies (Makedepend file)
#       TARGET - the name of the product of compilation (library or executable)
#       LIBRARY - the base name of the binary library file

##### Wrapup defs

TEMPOBJ = $(SRC:.cc=.o)
OBJ     = $(TEMPOBJ:.cc=.o)
DIRT    = $(OBJ) Makedepend
ARCHIVE = $(LIBRARY).a
DYLIB     = $(LIBRARY).dylib

default: dylib

archive: $(ARCHIVE)

dylib: $(DYLIB)

##### Delete all products of compilation and dependency list.

clean: dust
	@ $(RM) $(ARCHIVE) $(DYLIB)

##### Delete extraneous by-products of compilation.

dust:
	@ $(RM) $(DIRT)
	@ touch Makedepend

##### Rebuild the dependency list.
# NOTE: 'make' does not support automatic dependency updating like 'smake'

depend: $(SRC) $(INC)
	$(DEPEND) $(INCLUDES) -- $?
	@ echo

##### Rebuild an Emacs tags table (the TAGS file).

tags:	$(SVN_FILES)
	$(TAG) $?

##### Generate documentation

#doc: $(DOC)


##### Test Directory
# These targets apply to any directory that has a 'test' subdirectory.

## Build library and test directory
all: default test

## Build test directory
test: $(OBJ)
	@ if [ -d test  ]; \
		then \
	                cd test; $(MAKE) ; \
	  fi

## Clean module and test directories
cleanall: clean
	@ if [ -d test  ]; \
		then \
	                cd test; $(MAKE) clean; \
	  fi

## Dust module and test directories
dust-all: dust
	@ if [ -d test  ]; \
		then \
	                cd test; $(MAKE) dust; \
	  fi


##### Internal Targets  -- not typically invoked explicitly.

# Build an archive library (.a file)
# This will update an existing archive library with any object files newer
# than it, or create the library from existing objects if it does not exist.

$(ARCHIVE): $(OBJ)
	$(AR) -o $(ARCHIVE) $?

# KMD: need correct command line options!
## Build a shared library (DYLIB, .dylib file)
$(DYLIB): $(OBJ)
	$(CC) -o $(DYLIB) -shared $(OBJ)

##### SVN conveniences

# KMD: later
#include ../make/svn.make
