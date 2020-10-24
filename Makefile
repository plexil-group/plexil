# Top level Makefile for Plexil

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

SHELL = /bin/sh

# Check environment
# N.B.: dir leaves a trailing /
MAKEFILE_DIR := $(realpath $(join $(dir $(firstword $(MAKEFILE_LIST))),.))

ifeq ($(PLEXIL_HOME),)
PLEXIL_HOME := $(MAKEFILE_DIR)
else
ifneq ($(PLEXIL_HOME),$(MAKEFILE_DIR))
$(error Environment variable PLEXIL_HOME is in error. It must be set to $(MAKEFILE_DIR) before proceeding)
endif
endif

export PLEXIL_HOME

include makeinclude/standard-defs.make

#
# Locations for GNU autotools
#

# TODO? test for existence
AUTORECONF := autoreconf

# Primary target
plexil-default: tools

# The whole shooting match
everything: universalExec TestExec IpcAdapter UdpAdapter plexil-compiler plexilscript checker plexilsim pv robosim sample checkpoint

# Just the tools without the examples
tools: universalExec TestExec IpcAdapter UdpAdapter plexil-compiler plexilscript checker plexilsim pv

# Core facilities
essentials: universalExec TestExec IpcAdapter UdpAdapter plexil-compiler plexilscript checker plexilsim

#
# Standalone targets
#

checker: checker/global-decl-checker.jar

checker/global-decl-checker.jar:
	(cd checker && ant jar)

pv: viewers/pv/luv.jar

viewers/pv/luv.jar:
	(cd viewers/pv && ant jar)

plexil-compiler: jars/PlexilCompiler.jar

jars/PlexilCompiler.jar:
	(cd compilers/plexil && ant install)

plexilscript: jars/plexilscript.jar

jars/plexilscript.jar:
	(cd compilers/plexilscript && ant install)

.PHONY: checker/global-decl-checker.jar viewers/pv/luv.jar
.PHONY: jars/PlexilCompiler.jar jars/plexilscript.jar

#
# Targets which depend on the Automake targets below
#
checkpoint: utils
	$(MAKE) -C examples/checkpoint

robosim: ipc utils
	$(MAKE) -C examples/robosim

sample: universalExec
	$(MAKE) -C examples/sample-app

#
# Targets under the Automake build system
#
# Dependencies may be too messy to capture here
#

app-framework: lib/libPlexilAppFramework.$(SUFSHARE)

exec-core: lib/libPlexilExec.$(SUFSHARE)

GanttListener: lib/libGanttListener.$(SUFSHARE)

ipc: lib/libipc.a bin/central

IpcUtils: lib/libIpcUtils.$(SUFSHARE)

IpcAdapter: lib/libIpcAdapter.$(SUFSHARE)

Launcher: lib/libLauncher.$(SUFSHARE)

LuvListener: lib/libLuvListener.$(SUFSHARE)

PlanDebugListener: lib/libPlanDebugListener.$(SUFSHARE)

plexilsim: bin/simulator

pugixml: lib/libpugixml.$(SUFSHARE)

sockets: lib/libPlexilSockets.$(SUFSHARE)

TestExec: bin/TestExec

UdpAdapter: lib/libUdpAdapter.$(SUFSHARE)

universalExec: bin/universalExec 

utils: lib/libPlexilUtils.$(SUFSHARE)

value: lib/libPlexilValue.$(SUFSHARE)

bin/central lib/libIpc.a : most
bin/simulator : most
bin/TestExec : most
bin/universalExec : most
lib/libGanttListener.$(SUFSHARE) : most
lib/libipc.a : most
lib/libIpcAdapter.$(SUFSHARE) : most
lib/libIpcUtils.$(SUFSHARE) : most
lib/libLuvListener.$(SUFSHARE) : most
lib/libLauncher.$(SUFSHARE) : most
lib/libPlanDebugListener.$(SUFSHARE) : most
lib/libPlexilAppFramework.$(SUFSHARE) : most
lib/libPlexilExec.$(SUFSHARE) : most
lib/libPlexilExpr.$(SUFSHARE) : most
lib/libPlexilIntfc.$(SUFSHARE) : most
lib/libPlexilSockets.$(SUFSHARE) : most
lib/libPlexilUtils.$(SUFSHARE) : most
lib/libPlexilValue.$(SUFSHARE) : most
lib/libpugixml.$(SUFSHARE) : most
lib/libUdpAdapter.$(SUFSHARE) : most

most: most-build most-install

most-build: src/Makefile
	$(MAKE) -C src

most-install: most-build src/Makefile
	$(MAKE) -C src install

# At bootstrap time, values of these variables come from makeinclude/standard-defs.make
src/Makefile: src/configure
	cd ./src && ./configure --prefix="$(PREFIX)" --exec-prefix="$(EXEC_PREFIX)" \
 --bindir="$(BINDIR)" --includedir="$(INCLUDEDIR)" --libdir="$(LIBDIR)" \
 CC="$(CC)" CXX="$(CXX)" \
 CPPFLAGS="$(INITIAL_CPPFLAGS)" CFLAGS="$(INITIAL_CFLAGS)" CXXFLAGS="$(INITIAL_CXXFLAGS)" \
 --disable-static --enable-gantt --enable-ipc --enable-sas --enable-test-exec --enable-udp

#
# Bootstrapping autobuild files
#

# Must recreate configure if any of the Makefile.am files changes
MAKEFILE_AMS = $(wildcard src/**/Makefile.am)

# Create m4 directory - some older versions of autotools won't do it for us
src/configure: src/configure.ac $(MAKEFILE_AMS)
	cd ./src && mkdir -p m4 && $(AUTORECONF) -f -i

#
# End Automake targets
#

clean:: clean-examples
	-@$(MAKE) -C src $@ > /dev/null 2>&1
	@(cd compilers/plexil && ant $@)
	@(cd compilers/plexilscript && ant $@)
	@(cd checker && ant $@)
	@(cd jars && $(RM) plexilscript.jar)
	@(cd viewers/pv && ant $@)
	@$(RM) lib/lib* bin/* include/*
	@$(RM) examples/checkpoint/saves/*.xml
	@ echo Done.

clean-examples:
	-@$(MAKE) -C examples $@

# Clean up after autotools
distclean squeaky-clean: | clean
	@(cd compilers/plexil && ant uninstall)
	@(cd compilers/plexilscript && ant uninstall)
	@(cd src && $(RM) */Makefile */Makefile.in)
	@(cd src/apps && $(RM) */Makefile */Makefile.in)
	@(cd src/interfaces && $(RM) */Makefile */Makefile.in)
	@(cd src/third-party/ipc && $(RM) Makefile Makefile.in)
	@(cd src/third-party/pugixml/src && $(RM) Makefile Makefile.in)
	@(cd src && $(RM) Makefile Makefile.in aclocal.m4 \
 compile configure configure.env config.guess config.status config.sub \
 cppcheck.sh depcomp INSTALL install-sh libtool ltmain.sh missing \
 plexil-config.h plexil-config.h.in stamp-h1)
	@(cd src && $(RM) -rf m4 autom4te.cache)

# *** TODO: release target(s) ***

# Convenience targets

# Create an Emacs tags file capturing most relevant file types
#   Note: The indirection forces recreation of TAGS file in
#   MacOS, which is case-insensitive.
#
tags: alltags

ctags: 
	@ find . \( -name "*.cc" -or -name "*.cpp" -or -name "*.hh" -or -name "*.hpp" -or -name Makefile \) | etags -

jtags:
	@ find . \( -name "*.java" \) | etags -

alltags:
	@ find . \( -name "*.cc" -or -name "*.cpp" -or -name "*.hh" -or -name "*.hpp" -or -name "*.java" -or -name "*.xml" -or -name Makefile \) | etags -

.PHONY: app-framework exec-core GanttListener ipc IpcAdapter IpcUtils 

.PHONY: alltags clean ctags jtags most most-build most-install plexil-default squeaky-clean tags
