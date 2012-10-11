# Top level Makefile for Plexil

# Copyright (c) 2006-2012, Universities Space Research Association (USRA).
# All rights reserved.
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

all: TestExec UniversalExec IpcAdapter UdpAdapter GanttListener plexil-compiler checker plexilsim robosim sample

# convenience target for AMO project
amo AMO: exec-core app-framework luv plexil-compiler

TestExec: exec-core PlanDebugListener LuvListener luv
	$(MAKE) -C src/apps/TestExec

plexilsim: utils ipc IpcUtils
	$(MAKE) -C src/apps/StandAloneSimulator plexilsim

robosim: UniversalExec IpcAdapter
	$(MAKE) -C src/apps/robosim

sample: UniversalExec utils app-framework
	$(MAKE) -C src/apps/sample

universal-exec UniversalExec: exec-core app-framework
	$(MAKE) -C src/universal-exec

checker:
	(cd checker && ant jar)

luv:
	(cd luv && ant jar)

plexil-compiler:
	$(MAKE) -C compilers/plexil

pugixml:
	$(MAKE) -C src/third-party/pugixml/src

utils: pugixml
	$(MAKE) -C src/utils

exec-core: utils
	$(MAKE) -C src/exec

IpcAdapter: app-framework IpcUtils
	$(MAKE) -C src/interfaces/IpcAdapter

UdpAdapter: app-framework
	$(MAKE) -C src/interfaces/UdpAdapter

GanttListener: utils exec-core app-framework
	$(MAKE) -C src/interfaces/GanttListener

IpcUtils: ipc
	$(MAKE) -C src/interfaces/IpcUtils

LuvListener: exec-core sockets
	$(MAKE) -C src/interfaces/LuvListener

PlanDebugListener: exec-core
	$(MAKE) -C src/interfaces/PlanDebugListener

app-framework: exec-core sockets LuvListener PlanDebugListener
	$(MAKE) -C src/app-framework

sockets:
	$(MAKE) -C src/interfaces/Sockets

ipc:
	$(MAKE) -C src/third-party/ipc \
 PUBLIC_BIN_DIR=$(PLEXIL_HOME)/bin PUBLIC_LIB_DIR=$(PLEXIL_HOME)/lib PUBLIC_INC_DIR=$(PLEXIL_HOME)/include \
 THREADED=1 MAKE_SHARED_LIBS=1 SUBDIRS='etc src doc'

clean-ipc:
	-$(MAKE) -C src/third-party/ipc \
 PUBLIC_BIN_DIR=$(PLEXIL_HOME)/bin PUBLIC_LIB_DIR=$(PLEXIL_HOME)/lib PUBLIC_INC_DIR=$(PLEXIL_HOME)/include \
 clean
	-$(RM) lib/libipc.*

clean: clean-ipc
	-$(MAKE) -C src/third-party/pugixml/src $@
	-$(MAKE) -C src/utils $@
	-$(MAKE) -C src/exec $@
	-$(MAKE) -C src/interfaces/GanttListener $@
	-$(MAKE) -C src/interfaces/IpcAdapter $@
	-$(MAKE) -C src/interfaces/IpcUtils $@
	-$(MAKE) -C src/interfaces/LuvListener $@
	-$(MAKE) -C src/interfaces/PlanDebugListener $@
	-$(MAKE) -C src/interfaces/Sockets $@
	-$(MAKE) -C src/interfaces/UdpAdapter $@
	-$(MAKE) -C src/CORBA $@
	-$(MAKE) -C src/app-framework $@
	-$(MAKE) -C src/universal-exec $@
	-$(MAKE) -C src/apps/robosim $@
	-$(MAKE) -C src/apps/StandAloneSimulator $@
	-$(MAKE) -C src/apps/TestExec $@
	-$(MAKE) -C src/apps/sample $@
	(cd luv && ant $@)
	(cd checker && ant $@)
	(cd compilers/plexil && ant $@)
	-$(RM) lib/lib*
	@ echo Done.

# Convenience targets

#TestMultiExec: luv
#	(cd universal-exec; jam)
#	$(MAKE) -C interfaces all
#	(cd app-framework; jam)
#	(cd apps/TestMultiExec; jam)

# The following targets apply only when the UE is being used with an
# ACE/TAO Corba installation.

corba: utils exec-core app-framework
	$(MAKE) -C src/CORBA all

corba-utils: utils
	$(MAKE) -C src/CORBA $@

# Create an Emacs tags file capturing most relevant file types
#   Note: The indirection forces recreation of TAGS file in
#   MacOS, which is case-insensitive.
#
tags: alltags

ctags: 
	@ find . \( -name "*.cc" -or -name "*.hh" -or -name Makefile \) | etags -

jtags:
	@ find . \( -name "*.java" \) | etags -

alltags:
	@ find . \( -name "*.cc" -or -name "*.hh" -or -name "*.java" -or -name "*.xml" -or -name Makefile \) | etags -
