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

# Standard macro definitions for Plexil make files.

# N.B. This file serves two distinct purposes:
#  - to supply defaults to be used with src/configure;
#  - to establish the build environment for the Plexil examples.

# Note that these are defaults -
# overrides (if any) are defined in the platform-*.make files in this directory.

ifeq ($(PLEXIL_HOME),)
$(error The environment variable PLEXIL_HOME is not set. Exiting.)
endif

# If PLEXIL is already built, get the environment from results of 'configure'.
CONFIGURE_ENV := $(PLEXIL_HOME)/src/configure.env

ifneq ($(wildcard $(CONFIGURE_ENV)),)
# Get the info from the existing configure.env
include $(CONFIGURE_ENV)

# ... but in case we are reconfiguring:
INITIAL_CPPFLAGS	?= $(CONFIGURED_CPPFLAGS)
INITIAL_CFLAGS		?= $(CONFIGURED_CFLAGS)
INITIAL_CXXFLAGS	?= $(CONFIGURED_CXXFLAGS)

else 
#
# Set defaults to supply to 'configure'
# but allow them to be overridden in the environment
# 

##### C/C++ compiler options

##### *** N.B.: Most of these presume gcc,
##### *** but clang is default on OS X and the BSDs,
##### *** and cross-compilers could be anything.
##### *** Fortunately clang emulates gcc's option parsing.

# Install the products into the source tree by default.
PREFIX 		?= $(PLEXIL_HOME)
INCLUDEDIR  ?= $(PREFIX)/include
EXEC_PREFIX ?= $(PREFIX)
BINDIR      ?= $(EXEC_PREFIX)/bin
LIBDIR      ?= $(EXEC_PREFIX)/lib
# more?

# Use the platform's default compilers.
CC			?= cc
CXX			?= c++

# Sane defaults for compiler flags.
INITIAL_CPPFLAGS	?=
INITIAL_CFLAGS		?= -g -O2 -Wall
INITIAL_CXXFLAGS	?= $(INITIAL_CFLAGS) -std=c++03

# end defaults for configure
endif

#
# Variables autoconf should set for us.
# See also configure.env.in
#

# Commands
ETAGS		?= etags
LN_S		?= /bin/ln -s
SHELL       ?= /bin/sh

# Where build products should be installed.
# The defaults may be individually overridden by 'configure'.

ifeq ($(BINDIR),)
BINDIR		:= $(PREFIX)/bin
endif

ifeq ($(INCLUDEDIR),)
INCLUDEDIR	:= $(PREFIX)/include
endif

ifeq ($(LIBDIR),)
LIBDIR		:= $(PREFIX)/lib
endif



LIBRARY_SEARCH_PATH_FLAG	= -L
LIB_PATH_FLAGS				= $(LIBRARY_PATH_SEARCH_FLAG)$(LIBDIR)

INC_DIRS	:= $(INCLUDEDIR)

# FIXME: find way to integrate with 'configure', libtool settings
# Which variant(s) to build by default
# These can be overridden at the command line or in the shell environment
ifeq ($(PLEXIL_STATIC),)
PLEXIL_SHARED		?= 1
else
PLEXIL_SHARED		?=
endif

##### Other utilities

CP		?= /bin/cp
MKDIR	?= /bin/mkdir

DEPEND_FLAGS ?= -MM

# Include path

SYSTEM_INC_DIRS	?=
INCLUDES	= $(addprefix -isystem,$(SYSTEM_INC_DIRS)) $(addprefix -I,$(INC_DIRS))

# Compiler flags for shared libraries
POSITION_INDEPENDENT_CODE_FLAG	:= -fPIC

# Compiler flags for static linking
STATIC_FLAG := -static

##### Library support

# Names the library that will be the product of this make.
# User must set this to be useful.
LIBRARY		=

# Standard libraries for application framework based examples
UE_LIBS		= pugixml PlexilAppFramework PlexilXmlParser PlexilExec PlexilIntfc PlexilExpr PlexilValue PlexilUtils
EXTRA_LIBS	= 
LIB_FLAGS	= $(foreach lib,$(EXTRA_LIBS) $(UE_LIBS),-l$(lib))

##### Executable support

# Names the executable that will be the product of this make.
# User must set this to be useful.
EXECUTABLE	=

#
# Linker
#

# Linker program

# KMD: Command for building archive libraries, if applicable.
AR		= ar

# Command for building shared libraries, if applicable.
LD		= $(CXX) $(CXXFLAGS) $(foreach flag,$(EXE_FLAGS),$(LINKER_PASSTHROUGH_FLAG)$(flag))

# Compiler flag to pass an argument to the linker
LINKER_PASSTHROUGH_FLAG			?= -Wl,
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
# Linker flag to construct statically linked executable
STATIC_EXE_FLAG				:= -Bstatic

EXE_FLAGS				=
ifneq ($(PLEXIL_STATIC),)
EXE_FLAGS				+= $(STATIC_EXE_FLAG)
endif

##### Java

# Choose appropriate default version of Java
ifeq ($(JAVA_HOME),)
JAVA  ?= java
JAVAC ?= javac
JAR   ?= jar
else
JAVA  ?= $(JAVA_HOME)/bin/java
JAVAC ?= $(JAVA_HOME)/bin/javac
JAR   ?= $(JAVA_HOME)/bin/jar
endif

##### Conveniences

# Default for CVS targets in svn.make; should be shadowed to be more useful.
SVN_FILES       = *

##### A pre-emptive strike against some 3rd party platform include files

all: plexil-default

ifneq ($(PLEXIL_SHARED),)
ifneq ($(PLEXIL_STATIC),)
$(error PLEXIL_STATIC and PLEXIL_SHARED cannot both be true. Exiting.)
endif
endif

ifneq ($(PLEXIL_SHARED),)
# Emulate libtool
VARIANT_CPPFLAGS += -DPIC
VARIANT_CFLAGS   += $(POSITION_INDEPENDENT_CODE_FLAG)
VARIANT_CXXFLAGS += $(POSITION_INDEPENDENT_CODE_FLAG)
endif

ifneq ($(PLEXIL_STATIC),)
VARIANT_CPPFLAGS +=
VARIANT_CFLAGS   += $(STATIC_FLAG)
VARIANT_CXXFLAGS += $(STATIC_FLAG)
endif

# FIXME: User supplied flags should override defaults.
CPPFLAGS    = $(CONFIGURED_CPPFLAGS) $(VARIANT_CPPFLAGS) $(INCLUDES)
CFLAGS		= $(CONFIGURED_CFLAGS) $(VARIANT_CFLAGS)
CXXFLAGS	= $(CONFIGURED_CXXFLAGS) $(VARIANT_CXXFLAGS)

include $(PLEXIL_HOME)/makeinclude/platform-defs.make

