# Standard macro definitions for Plexil make files.
# Note that these are defaults -
# overrides (if any) are defined in the platform-*.make files in this directory.

ifeq ($(PLEXIL_HOME),)
$(error The environment variable PLEXIL_HOME is not set. Exiting.)
endif

# Which variant(s) to build by default
# These can be overridden at the command line or in the shell environment
PLEXIL_DEBUG		?= 1
PLEXIL_OPTIMIZED	?=
PLEXIL_PROFILE		?=
PLEXIL_SHARED		?= 1
PLEXIL_STATIC		?=

ifneq ($(PLEXIL_SHARED),)
ifneq ($(PLEXIL_STATIC),)
$(error PLEXIL_STATIC and PLEXIL_SHARED cannot both be true. Exiting.)
endif
endif

##### Basic utilities and Unix commands

SHELL           = /bin/sh

# Delete files (recursively, forced)
RM		= /bin/rm -fr
# File system link
LN		= /bin/ln -s
# Directory list
LS              = /bin/ls
DEPEND		= makedepend -f Makedepend
TAG		= etags -t
# Move a file
MV              = /bin/mv
# Make a directory
MKDIR           = /bin/mkdir
# Copy a file
CP              = /bin/cp -p

# Define when purify is available.  When it is desired, edit the file
# src/main/targets.make, as instructed there.
PURIFY		= purify

# Which compilers to use by default.
CC	        := gcc
CXX	        := g++

##### C++ compiler options.

# KMD: Need proper flags for warning suppression...
# Note: we do not normally skip any warnings.  The exceptions for now are:
# 1355: extra trailing ';'
# 1009: A "/*" appears inside a comment.
# 3322: omission of explicit return type (pervasive in X11 files)
# 1356: nonstandard member constant declaration
# 1401: qualified name in class member declaration
# 1375: base class destructor not virtual
# 1201: trailing comma
# 1234: "access control not specified (public by default)" -- found in some STL
# SKIP_WARNINGS = -woff 1355,1009,3322,1356,1375,1401,1201,1234

# Compiler options
#STANDARD_CFLAGS		= -multigot $(SKIP_WARNINGS)
STANDARD_CFLAGS		:=
STANDARD_CXXFLAGS	:= -DTIXML_USE_STL

# Include path
INC_DIRS	= .
INCLUDES	= $(foreach incdir,$(INC_DIRS),-I$(incdir))

# Compiler flags for shared libraries
POSITION_INDEPENDENT_CODE_FLAG	:= -fPIC

# Compiler flags for debug builds
DEBUG_FLAGS	:= -gfull -g3
WARNING_FLAGS	:= -Wall

# Compiler flags for profiling
PROFILE_FLAGS	:= -pg

# Compiler flags for optimized builds
OPTIMIZE_FLAGS	:= -O3 -DPLEXIL_FAST

# Compiler flags for code coverage (e.g., gcov)
COVERAGE_FLAGS	:= -fprofile-arcs -ftest-coverage

VARIANT_CFLAGS	=
ifneq ($(PLEXIL_DEBUG),)
VARIANT_CFLAGS	+= $(DEBUG_FLAGS)
endif
ifneq ($(PLEXIL_OPTIMIZE),)
VARIANT_CFLAGS	+= $(OPTIMIZE_FLAGS)
endif
ifneq ($(PLEXIL_PROFILE),)
VARIANT_CFLAGS	+= $(PROFILE_FLAGS)
endif
ifneq ($(PLEXIL_SHARED),)
VARIANT_CFLAGS	+= $(POSITION_INDEPENDENT_CODE_FLAGS)
endif

CFLAGS		+= $(STANDARD_CFLAGS) $(VARIANT_CFLAGS) $(INCLUDES)
CXXFLAGS	+= $(STANDARD_CXXFLAGS) $(VARIANT_CFLAGS) $(INCLUDES)

##### Library support

# Default name for library.  Must be redefined to be useful.
LIBRARY		= 

LIB_PATH	=
LIB_PATH_FLAGS	= $(foreach libdir,$(LIB_PATH),$(LIBRARY_PATH_SEARCH_FLAG)$(libdir))
LIBS		=
LIB_FLAGS	= $(foreach lib,$(LIBS),-l$(lib))

#
# Linker
#

# Linker program

# KMD: Command for building archive libraries, if applicable.
AR		= $(CXX) -ar 

# Command for building shared libraries, if applicable.
LD		= $(CXX)

# Compiler flag to pass an argument to the linker
LINKER_PASSTHROUGH_FLAG			:= -Wl,
# Linker flag for link-time library search path
LIBRARY_PATH_SEARCH_FLAG		:= -L
LINKTIME_SHARED_LIBRARY_PATH_FLAG	:= -L
# Linker flag for static link-time library search path
STATIC_LIBRARY_PATH_FLAG		:= -L
# Linker flag for run-time library search path
RUNTIME_SHARED_LIBRARY_PATH_FLAG	:= -rpath
# Linker flag to construct shared library
SHARED_FLAGS				:= -shared
# Extension for shared library
SUFSHARE				:= .so
# Name of system library with pthreads API
PTHREADS_LIBRARY			:= pthread
# Name of system library with dynamic loading API
DLOPEN_LIBRARY				:= dl

##### Conveniences

# Default for CVS targets in svn.make; should be shadowed to be more useful.
SVN_FILES       = *

##### Suffix rules

# KMD: determine if useful
# .SUFFIXES : .cc .hh .o .c .h

include $(PLEXIL_HOME)/makeinclude/platform-defs.make
