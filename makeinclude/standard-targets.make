# Copyright (c) 2006-2020, Universities Space Research Association (USRA).
#  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Universities Space Research Association nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Universal targets for Plexil Makefiles.  This is not a standalone make file,
# and must be included to complete a full make file.

# This file assumes the following macros are properly defined prior to
# this file's inclusion.  Some are the programmer's responsibility, and some
# are found in "standard-defs.make"
#
#       INC - .H files
#       SRC - .C files
#       ETAGS - the command to create an Emacs tags table
#       SVN_FILES - a list of all files under SVN control
#       INCLUDES - "-I" flags
#       RM - command to remove files (GNU make defaults to 'rm -f')
#       DEPEND_FLAGS - compiler option(s) to generate header dependencies
#       LIBRARY - the base name of the binary library file
#       EXECUTABLE - the name of the executable file
#		EXTRA_LIBS - libraries beyond the standard Plexil core libraries

##### Wrapup defs

# This works for any file suffix, e.g. .c, .cc, .cpp, .C, ...
OBJ     = $(addsuffix .o,$(basename $(SRC)))
DEPS    = $(addsuffix .d,$(basename $(SRC)))
DEP_TMP	= $(wildcard $(addsuffix .*,$(DEPS)))
DIRT    = $(OBJ) $(DEPS) $(DEP_TMP)

# Macro for libraries
define library
$(1)_OBJ := $$(addsuffix .o,$$(basename $$($(1)_SRC)))
$(1)_DEPS := $$(addsuffix .d,$$(basename $$($(1)_SRC)))
DEPS += $$($(1)_DEPS)
DIRT += $$($(1)_OBJ) $$($(1)_DEPS)

ifneq ($$(PLEXIL_SHARED),)
$(1)_SHLIB := lib$(1)$(SUFSHARE)

plexil-default: $$(LIBDIR)/$$($(1)_SHLIB)

$$(LIBDIR)/$$($(1)_SHLIB) : $$($(1)_SHLIB) $$(LIBDIR)
	$$(CP) $$< $$@

$$($(1)_SHLIB): $$(LIBDIR) $$($(1)_OBJ)
	-$$(RM) $$@
	$$(LD) $$(SHARED_FLAGS) $$(EXTRA_LD_SO_FLAGS) $$(EXTRA_FLAGS) -o $$@ $$($(1)_OBJ) $$(LIB_PATH_FLAGS) $$(LIB_FLAGS) $$(LIBS)

localclean::
	@$$(RM) $$($(1)_SHLIB)
endif

ifneq ($$(PLEXIL_STATIC),)
$(1)_ARCHIVE := lib$(1).a

plexil-default: $$(LIBDIR)/$$($(1)_ARCHIVE)

$$(LIBDIR)/$$($(1)_ARCHIVE): $$($(1)_ARCHIVE) $$(LIBDIR)
	$$(CP) $$< $$@

$$($(1)_ARCHIVE): $$(LIBDIR) $($(1)_OBJ)
	$$(AR) crus $$@ $$($(1)_OBJ)

localclean::
	@$$(RM) $$($(1)_ARCHIVE)
endif

# Library includes to install
ifneq ($$($(1)_INC),)
plexil-default: $$(INCDIR)/$$($(1)_INC)
$$(INCDIR)/$$($(1)_INC): $$(INCDIR) $$($(1)_INC)
	$$(CP) $$($(1)_INC) $$<
endif

endef

ifneq ($(LIBRARIES),)
$(LIBDIR):
	mkdir -p $(LIBDIR)

$(foreach lib,$(LIBRARIES),$(eval $(call library,$(lib))))
endif

# Macro for executables

define executable
$(1)_OBJ := $$(addsuffix .o,$$(basename $$($(1)_SRC)))
$(1)_DEPS := $$(addsuffix .d,$$(basename $$($(1)_SRC)))
DEPS += $$($(1)_DEPS)
DIRT += $$($(1)_OBJ) $$($(1)_DEPS)

plexil-default: $$(BINDIR)/$1

$$(BINDIR)/$1: $1 $$(BINDIR)
	$$(CP) $$< $$@

$1: $$($(1)_OBJ)
	$$(LD) $$(EXTRA_EXE_FLAGS) $$(EXTRA_FLAGS) -o $1 $$($(1)_OBJ) $$(LIB_PATH_FLAGS) $$(LIB_FLAGS) $$(LIBS)

localclean::
	@$$(RM) $1 $1.dSYM

endef

ifneq ($(EXECUTABLE),)
$(BINDIR):
	mkdir -p $(BINDIR)

$(foreach exec,$(EXECUTABLE),$(eval $(call executable,$(exec))))
endif

##### Delete extraneous by-products of compilation.

localdust::
	$$(RM) $(DIRT)

##### Dependencies

## Derived from the GNU make manual
%.d : %.c
	@$(RM) $@; \
	 if $(CC) $(DEPEND_FLAGS) $(CPPFLAGS) $< > $@.$$$$ 2> /dev/null; \
     then sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; fi; \
     $(RM) $@.$$$$

%.d : %.cc
	@$(RM) $@; \
	 if $(CXX) $(DEPEND_FLAGS) $(CPPFLAGS) $< > $@.$$$$ 2> /dev/null; \
     then sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; fi; \
     $(RM) $@.$$$$

%.d : %.cpp
	@$(RM) $@; \
	 if $(CXX) $(DEPEND_FLAGS) $(CPPFLAGS) $< > $@.$$$$ 2> /dev/null; \
     then sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; fi; \
     $(RM) $@.$$$$

# does anyone use .C for C++?

##### Rebuild an Emacs tags table (the TAGS file).

tags:	$(SVN_FILES)
	$(ETAGS) $?

##### Generate documentation

#doc: $(DOC)

##### Test Directory
# These targets apply to any directory that has a 'test' subdirectory.

## Build library and test directory
all: plexil-default

## Build test directory
test: plexil-default
	@ if [ -d test ]; \
	then \
		$(MAKE) -C test; \
	fi

## Clean module and test directories
clean:: dust localclean
	@ if [ -d test ]; \
	then \
		$(MAKE) -C test $@; \
	fi

## Dust module and test directories
dust:: localdust
	@ if [ -d test ]; \
	then \
		$(MAKE) -C test $@; \
	fi

.PHONY: dust clean localclean localdust

ifeq ($(findstring $(MAKECMDGOALS),dust clean localdust localclean),)
-include $(DEPS)
endif
