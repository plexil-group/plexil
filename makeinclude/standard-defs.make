# Copyright (c) 2006-2009, Universities Space Research Association (USRA).
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

# Standard macro definitions for Plexil make files.
# Note that these are defaults -
# overrides (if any) are defined in the platform-*.make files in this directory.

ifeq ($(PLEXIL_HOME),)
$(error The environment variable PLEXIL_HOME is not set. Exiting.)
endif

# The location we're building into - may be overridden
TOP_DIR ?= $(PLEXIL_HOME)

# Which variant(s) to build by default
# These can be overridden at the command line or in the shell environment
PLEXIL_DEBUG		?= 1
PLEXIL_OPTIMIZED	?=
PLEXIL_PROFILE		?=
PLEXIL_SHARED		?= 1
PLEXIL_STATIC		?=

##### Basic utilities and Unix commands

SHELL           = /bin/sh

# Delete files (recursively, forced)
RM		= /bin/rm -fr
# File system link
LN		= /bin/ln -s
# Directory list
LS              = /bin/ls
DEPEND		= $(CXX) -MM
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

# Get compiler version - presumes gcc
CXX_VERSION := $(shell $(CXX) -dumpversion)
CXX_MAJOR_VERSION := $(firstword $(subst ., ,$(CXX_VERSION)))
CXX_MINOR_VERSION := $(word 2,$(subst ., ,$(CXX_VERSION)))
CXX_PATCH_VERSION := $(word 3,$(subst ., ,$(CXX_VERSION)))
# for debug use
#CXX_VERSION_DUMMY := $(info C++ version is $(CXX_MAJOR_VERSION) $(CXX_MINOR_VERSION) $(CXX_PATCH_VERSION))


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

# Defines
# -DTIXML_USE_STL ensures that any inclusion of tinyxml.h gets the STL types.
# -D__STDC_LIMIT_MACROS directs system include file stdint.h to define the C99 INTnn_MAX/MIN macros.
DEFINES			:= -DTIXML_USE_STL -D__STDC_LIMIT_MACROS

#STANDARD_CFLAGS		= -multigot $(SKIP_WARNINGS)
STANDARD_CFLAGS		:=
STANDARD_CXXFLAGS	:=

# Include path

SYSTEM_INC_DIRS	=
INC_DIRS	= .
INCLUDES	= $(addprefix -isystem,$(SYSTEM_INC_DIRS)) $(addprefix -I,$(INC_DIRS))

# Compiler flags for shared libraries
POSITION_INDEPENDENT_CODE_FLAG	:= -fPIC

# Compiler flags for debug builds
DEBUG_FLAGS	:= -g3
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

CFLAGS		+= $(DEFINES) $(STANDARD_CFLAGS) $(VARIANT_CFLAGS) $(INCLUDES)
CXXFLAGS	+= $(DEFINES) $(STANDARD_CXXFLAGS) $(VARIANT_CFLAGS) $(INCLUDES)

##### Library support

# Names the library that will be the product of this make.
# User must set this to be useful.
LIBRARY		=

LIB_PATH	:= $(PLEXIL_HOME)/lib
ifneq ($(PLEXIL_HOME),$(TOP_DIR))
LIB_PATH	+= $(TOP_DIR)/lib
endif
LIB_PATH_FLAGS	= $(foreach libdir,$(LIB_PATH),$(LIBRARY_PATH_SEARCH_FLAG)$(libdir))
LIBS		=
LIB_FLAGS	= $(foreach lib,$(LIBS),-l$(lib))

##### Executable support

# Names the executable that will be the product of this make.
# User must set this to be useful.
EXECUTABLE	=

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

##### A pre-emptive strike against some 3rd party platform include files

all: plexil-default

include $(PLEXIL_HOME)/makeinclude/platform-defs.make

# Check here in case some platform include file (re)defines these

ifneq ($(PLEXIL_SHARED),)
ifneq ($(PLEXIL_STATIC),)
$(error PLEXIL_STATIC and PLEXIL_SHARED cannot both be true. Exiting.)
endif
endif

ifneq ($(PLEXIL_SHARED),)
VARIANT_CFLAGS	+= $(POSITION_INDEPENDENT_CODE_FLAG)
endif
