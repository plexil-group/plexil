# Top level Makefile for Plexil

# Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

MY_PLEXIL_HOME := $(shell pwd)
ifneq ($(PLEXIL_HOME),)
ifneq ($(PLEXIL_HOME),$(MY_PLEXIL_HOME))
$(error Environment variable PLEXIL_HOME is in error. It must be set to $(MY_PLEXIL_HOME) before proceeding)
endif
endif

export PLEXIL_HOME := $(MY_PLEXIL_HOME)

default: all

# Add robosim later
all: UniversalExec TestExec IpcAdapter UdpAdapter plexil-compiler plexilscript checker plexilsim robosim sample pv

robosim: 
	$(MAKE) -C examples/robosim

sample:
	$(MAKE) -C examples/sample-app

checker:
	(cd checker && ant jar)

pv:
	(cd viewers/pv && ant jar)

plexil-compiler:
	$(MAKE) -C compilers/plexil

plexilscript:
	(cd compilers/plexilscript && ant install)

app-framework exec-core GanttListener ipc IpcUtils IpcAdapter LuvListener PlanDebugListener plexilsim pugixml sockets TestExec UdpAdapter universal-exec UniversalExec utils: src/Makefile
	$(MAKE) -C src
	$(MAKE) -C src install

src/Makefile: src/configure
    cd src && ./configure --prefix=$(PLEXIL_HOME) --disable-static --enable-gantt --enable-ipc --enable-sas --enable-test-exec --enable-udp

clean:
	-$(MAKE) -C compilers/plexil $@
	-$(MAKE) -C examples/robosim $@
	-$(MAKE) -C examples/sample-app $@
	-$(MAKE) -C src $@
	(cd checker && ant $@)
	(cd compilers/plexilscript && ant $@)
	(cd viewers/pv && ant $@)
	-$(RM) lib/lib*
	@ echo Done.

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
