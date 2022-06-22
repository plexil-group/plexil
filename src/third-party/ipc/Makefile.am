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

# Wrapper Makefile for TCA-IPC build

# There are 3 distinct locations we care about:
#  - The source directory;
#  - The build directory, where generated files live (e.g. objects);
#  - The install directory, where the user-visible products are placed.
# For in-tree builds, source and build are the same location
# For out-of-tree builds, they are distinct.
# The IPC GNUmakefiles previously did not distinguish between them.

IPC_LOCATIONS = BASE_DIR=$(abs_srcdir) BUILD_DIR=$(abs_builddir) \
 INSTALL_DIR=$(prefix) PUBLIC_BIN_DIR=$(bindir) PUBLIC_LIB_DIR=$(libdir) PUBLIC_INC_DIR=$(includedir)

IPC_SUBDIRS = SUBDIRS='src'

IPC_OPTIONS = 
if THREADS_OPT
IPC_OPTIONS += THREADED=1
endif
if STATIC_LIB_OPT
IPC_OPTIONS += MAKE_STATIC_LIBS=1
endif
if SHARED_LIB_OPT
IPC_OPTIONS += MAKE_SHARED_LIBS=1
endif

all-local: ipc-all install-exec-local

ipc-all:
	$(MAKE) -C $(srcdir) -f GNUmakefile.ipc $(AM_MAKEFLAGS) \
 $(IPC_LOCATIONS) $(IPC_OPTIONS) $(IPC_SUBDIRS) all

clean-local:
	$(MAKE) -C $(srcdir) -f GNUmakefile.ipc $(AM_MAKEFLAGS) \
 $(IPC_LOCATIONS) $(IPC_OPTIONS) $(IPC_SUBDIRS) clean

install-exec-local:
	$(MAKE) -C $(srcdir) -f GNUmakefile.ipc $(AM_MAKEFLAGS) \
 $(IPC_LOCATIONS) $(IPC_OPTIONS) $(IPC_SUBDIRS) install_incs install_libs install_bins

install-data-local:
	$(MAKE) -C $(srcdir) -f GNUmakefile.ipc $(AM_MAKEFLAGS) \
 $(IPC_LOCATIONS) $(IPC_OPTIONS) $(IPC_SUBDIRS) install_mans install_docs

uninstall-local:
	$(MAKE) -C $(srcdir) -f GNUmakefile.ipc $(AM_MAKEFLAGS) \
 $(IPC_LOCATIONS) $(IPC_OPTIONS) $(IPC_SUBDIRS) uninstall

# Distribute files from here.
#EXTRA_DIST = subdir/Makefile subdir/program.c ...

EMPTY_AUTOMAKE_TARGETS = dvi pdf ps info html ctags check-local

.PHONY: $(EMPTY_AUTOMAKE_TARGETS)

$(EMPTY_AUTOMAKE_TARGETS):
