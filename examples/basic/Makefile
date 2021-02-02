# Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

# Peforms batch translations of basic examples into Core PLEXIL and Plexilscript.

SHELL = /bin/sh

MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
PLEXIL_HOME ?= $(dir $(dir $(MAKEFILE_DIR)))

export PLEXIL_HOME

PLEXIL_COMPILER = $(PLEXIL_HOME)/scripts/plexilc
PLEXILC_OPTS    = -p -q

PLANS_PLE      = $(wildcard *.ple) 
PLANS_PLX      = $(PLANS_PLE:.ple=.plx) 

SCRIPTS_PST    = $(wildcard scripts/*.pst)
SCRIPTS_PSX    = $(SCRIPTS_PST:.pst=.psx)

all: plans scripts

plans: $(PLANS_PLX)

scripts: $(SCRIPTS_PSX)

clean:
	@$(RM) -f $(PLANS_PLX) $(SCRIPTS_PSX)

.SUFFIXES: .ple .plx

%.plx: %.ple
	@$(PLEXIL_COMPILER) $(PLEXILC_OPTS) $<

%.psx: %.pst
	@$(PLEXIL_COMPILER) $(PLEXILC_OPTS) $<

.PHONY: clean all
