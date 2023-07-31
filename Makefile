# Top level Makefile for Plexil
# Presumes GNU make

# Copyright (c) 2006-2023, Universities Space Research Association (USRA).
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

SHELL := /bin/sh

# Check environment
# N.B.: dir leaves a trailing /
MAKEFILE_DIR := $(realpath $(join $(dir $(firstword $(MAKEFILE_LIST))),.))

ifneq ($(PLEXIL_HOME),$(MAKEFILE_DIR))
PLEXIL_HOME := $(MAKEFILE_DIR)
$(info Setting environment variable PLEXIL_HOME to $(MAKEFILE_DIR))
endif

export PLEXIL_HOME

# Files in git submodules
SUBMODULES := src/third-party/pugixml/src

include makeinclude/standard-defs.make

#
# Locations for GNU autotools
#

# Have to do it this way because 'command' is a shell builtin; the
# straightforward '$(shell command -v autoreconf)' fails with the message
# 'sh: command: not found'.
AUTORECONF := $(shell echo 'command -v autoreconf' | /bin/sh)

# Primary target
plexil-default: tools

# The whole shooting match
everything: universalExec TestExec IpcAdapter UdpAdapter plexil-compiler plexilscript checker plexilsim pv robosim sample checkpoint

# Just the tools without the examples
tools: universalExec TestExec IpcAdapter UdpAdapter plexil-compiler plexilscript checker plexilsim pv

# Core facilities
essentials: universalExec TestExec IpcAdapter UdpAdapter plexil-compiler plexilscript checker plexilsim

#
# Java targets
#

checker: checker/global-decl-checker.jar

checker/global-decl-checker.jar:
	(cd checker && ant jar)

.PHONY: checker/global-decl-checker.jar

pv: viewers/pv/luv.jar

viewers/pv/luv.jar:
	(cd viewers/pv && ant jar)

.PHONY: viewers/pv/luv.jar

plexil-compiler: jars/PlexilCompiler.jar

jars/PlexilCompiler.jar:
	(cd compilers/plexil && ant install)

plexilscript: jars/plexilscript.jar

jars/plexilscript.jar:
	(cd compilers/plexilscript && ant install)

.PHONY: jars/PlexilCompiler.jar jars/plexilscript.jar

#
# ** TODO ** Documentation
#

# doc: doc/html/index.html

# doc/html/index.html:
# 	$(MAKE) -C doc

#.PHONY: doc/html/index.html

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
# If already configured, they should come from src/configure.env
src/Makefile: src/configure $(SUBMODULES)
	cd ./src && ./configure --prefix="$(PREFIX)" --exec-prefix="$(EXEC_PREFIX)" \
 --bindir="$(BINDIR)" --includedir="$(INCLUDEDIR)" --libdir="$(LIBDIR)" \
 CC="$(CC)" CXX="$(CXX)" \
 CPPFLAGS="$(INITIAL_CPPFLAGS)" CFLAGS="$(INITIAL_CFLAGS)" CXXFLAGS="$(INITIAL_CXXFLAGS)" \
 --disable-static --enable-ipc --enable-sas --enable-test-exec --enable-udp

#
# Bootstrapping
#

# Ensure that submodules have been cloned as well
$(SUBMODULES):
	git submodule update --init --recursive

# Must recreate configure if any of the Makefile.am files changes
MAKEFILE_AMS := $(shell find src -name Makefile.am -print)
AUTOMAKE_DIRS := $(dir $(MAKEFILE_AMS))

# Create m4 directory - some older versions of autotools won't do it for us
src/configure: src/configure.ac $(MAKEFILE_AMS)
	(cd ./src && mkdir -p m4 && $(AUTORECONF) -f -i)

#
# Cleaning targets
#

# Overall cleanup
clean:: clean-examples clean-exec-all
	@(cd compilers/plexil && ant $@) > /dev/null 2>&1
	-@$(MAKE) -C compilers/plexil/test $@ > /dev/null 2>&1
	@(cd compilers/plexilscript && ant $@) > /dev/null 2>&1
	@(cd checker && ant $@) > /dev/null 2>&1
	-@test/TestExec-regression-test/cleanup-test-files $@
	@(cd viewers/pv && ant $@) > /dev/null 2>&1

clean-examples:
	-@$(MAKE) -C examples/checkpoint clean > /dev/null 2>&1
	-@$(MAKE) -C examples/multi-exec/udp clean > /dev/null 2>&1
	-@$(MAKE) -C examples/robosim clean > /dev/null 2>&1
	-@$(MAKE) -C examples/sample-app clean > /dev/null 2>&1

# Clean just the Exec build directories
clean-exec:
	-@$(MAKE) -C src clean > /dev/null 2>&1

# Clean up all files generated by configure
clean-exec-configuration: clean-exec
	@for dir in $(AUTOMAKE_DIRS) ; do (cd "$$dir" && $(RM) Makefile) ; done
	@(cd src && $(RM) config.log config.status configure.env plexil-config.h stamp-h1)

# Clean up after autoreconf, automake, libtoolize, etc.
clean-exec-all: clean-exec-configuration
	@for dir in $(AUTOMAKE_DIRS) ; do (cd "$$dir" && $(RM) Makefile.in) ; done
	@(cd src && $(RM) aclocal.m4 compile configure configure~ \
 cppcheck.sh config.guess config.sub depcomp INSTALL install-sh \
 libtool ltmain.sh missing plexil-config.h.in plexil-config.h.in.~)
	@(cd src && $(RM) -r m4 autom4te.cache)

.PHONY: clean clean-examples clean-exec clean-exec-configuration clean-exec-all

# Uninstall all programs and libraries
uninstall:
	@$(RM) $(LIBDIR)/lib* $(BINDIR)/* $(INCLUDEDIR)/*
	@(cd compilers/plexil && ant $@) > /dev/null 2>&1
	@(cd compilers/plexilscript && ant $@) > /dev/null 2>&1

# Restore the source tree to a just-checked-out state
distclean squeaky-clean: uninstall clean
	@(cd schema/validator && $(RM) -r .venv)
	@(cd compilers/plexil && ant uninstall) > /dev/null 2>&1
	@(cd compilers/plexilscript && ant uninstall) > /dev/null 2>&1

.PHONY: uninstall distclean squeaky-clean

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

.PHONY: app-framework exec-core ipc IpcAdapter IpcUtils

.PHONY: alltags ctags jtags most most-build most-install plexil-default tags uninstall
