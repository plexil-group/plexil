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

# This works for any file suffix, e.g. .c, .cc, .cpp, .C, ...
OBJ     = $(addsuffix .o,$(basename $(SRC)))
DIRT    = $(OBJ)
ARCHIVE = lib$(LIBRARY).a
SHLIB	= lib$(LIBRARY)$(SUFSHARE)

ifneq ($(LIBRARY),)
ifneq ($(PLEXIL_SHARED),)
default: shlib
endif
ifneq ($(PLEXIL_STATIC),)
default: archive
endif
else ifeq ($(EXECUTABLE),)
$(error Neither LIBRARY nor EXECUTABLE supplied for this Makefile. Exiting.)
endif

ifneq ($(EXECUTABLE),)
default: executable
endif

archive $(PLEXIL_HOME)/lib/$(ARCHIVE): $(ARCHIVE)
	$(CP) $(ARCHIVE) $(PLEXIL_HOME)/lib/

shlib $(PLEXIL_HOME)/lib/$(SHLIB): $(SHLIB)
	$(CP) $(SHLIB) $(PLEXIL_HOME)/lib/

executable $(PLEXIL_HOME)/bin/$(EXECUTABLE): $(EXECUTABLE)
	$(CP) $(EXECUTABLE) $(PLEXIL_HOME)/bin/

##### Delete all products of compilation and dependency list.

localclean: localdust
	-$(RM) $(ARCHIVE) $(SHLIB) $(EXECUTABLE) Makedepend

##### Delete extraneous by-products of compilation.

localdust:
	$(RM) $(DIRT)

##### Rebuild the dependency list.
# NOTE: 'make' does not support automatic dependency updating like 'smake'

depend: Makedepend

Makedepend: $(SRC) $(INC)
	$(DEPEND) $(INCLUDES) $(SRC)
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
test: default
	@ if [ -d test ]; \
	then \
		$(MAKE) -C test ; \
	fi

## Clean module and test directories
clean: localclean
	@ if [ -d test ]; \
	then \
		$(MAKE) -C test clean; \
	fi

## Dust module and test directories
dust: localdust
	@ if [ -d test ]; \
	then \
		$(MAKE) -C test dust; \
	fi


##### Internal Targets  -- not typically invoked explicitly.

# Build an archive library (.a file)
# This will update an existing archive library with any object files newer
# than it, or create the library from existing objects if it does not exist.

$(ARCHIVE): depend $(OBJ)
	$(AR) -o $(ARCHIVE) $?

## Build a shared library (SHLIB)
$(SHLIB): depend $(OBJ)
	$(LD) $(SHARED_FLAGS) $(LIB_PATH_FLAGS) $(LIB_FLAGS) -o $(SHLIB) $(OBJ)

## Build an executable
$(EXECUTABLE): depend $(OBJ)
	$(LD) $(LIB_PATH_FLAGS) $(LIB_FLAGS) -o $(EXECUTABLE) $(OBJ)

##### SVN conveniences

# KMD: later
#include ../make/svn.make
