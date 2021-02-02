# -*- Mode: Makefile -*-
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

# File: $SVNROOT/makeinclude/generic-plexil.make
# Note: Generic makefile for plexilisp and standard plexil.  Compiles
#       and validates *.pl{i,e} and lib/*.pl{i,e}.

# For use in running agents from a makefile which includes this one
RUN_AGENTS = $(PLEXIL_HOME)/scripts/run-agents

# Find the .pli and .ple plans in ./ and lib/
PLANS = $(wildcard *.epx *.ple *.plp *.pli lib/*.epx lib/*.ple lib/*.plp lib/*.pli)

# Find all of the .plx targets implied by PLANS
TARGETS = $(filter %.plx, (PLANS:%.epx=%.plx) $(PLANS:%.ple=%.plx)(PLANS:%.plp=%.plx) $(PLANS:%.pli=%.plx))

# The default target for this file (leave all for others)
plx: $(TARGETS)

# Default clean targets.  Can be redefined in makefile which includes this file.
# This idiom allows makefiles which include this one to add more rules to clean, etc.
DIRS = . lib test

_dust:
	-@for dir in $(DIRS) ; do $(RM) $${dir}/*.{epx,last} ; done
dust:: _dust

_clean: dust
	-@for dir in $(DIRS) ; do $(RM) $${dir}/*.plx ; $(RM) core.* ; done
clean:: _clean

_cleaner: clean
	-@for dir in $(DIRS) ; do $(RM) $${dir}/*~ ; done
cleaner: _cleaner

plexil-targets:
	@echo "plans:   " $(PLANS)
	@echo "targets: " $(TARGETS)

##### PLEXIL Compiler defaults

PLEXIL_COMPILER = $(PLEXIL_HOME)/scripts/plexilc
PLEXILC_OPTS    = -p

##### Default pattern rules for generating Plexil XML

%.plx: %.ple
	$(PLEXIL_COMPILER) $(PLEXILC_OPTS) $<

%.plx: %.pli
	$(PLEXIL_COMPILER) $(PLEXILC_OPTS) $<

%.plx: %.plp
	$(PLEXIL_COMPILER) $(PLEXILC_OPTS) $<

%.plx: %.epx
	$(PLEXIL_COMPILER) $(PLEXILC_OPTS) $<

%.psx: %.pst
	$(PLEXIL_COMPILER) $(PLEXILC_OPTS) $<

.PHONY: _dust dust _clean clean _cleaner cleaner plexil-targets

# EOF
