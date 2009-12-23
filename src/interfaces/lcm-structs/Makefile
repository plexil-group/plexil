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


include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= ComStructs

STRUCTS		:= $(wildcard *.lcm)
STRUCTS_C	:= $(STRUCTS:%.lcm=%.c)
STRUCTS_H	:= $(STRUCTS:%.lcm=%.h)
STRUCTS_O	:= $(STRUCTS:%.lcm=%.o)

SRC		:= $(STRUCTS_C)
INC		:=

SVN_FILES	:= Makefile $(STRUCTS)

LIBS		:= lcm

DIRT		+= $(STRUCTS_C)

ifndef LCM_PATH
LCM_PATH	:= $(PLEXIL_HOME)/third-party/lcm
endif

#LCMGEN		:= $(PLEXIL_HOME)/bin/lcm-gen
LCMGEN		:= $(LCM_PATH)/lcmgen/lcm-gen

INC_DIRS	:= $(LCM_PATH)
LIB_PATH	+= $(LCM_PATH)/lcm/.libs

all: default

# Pattern rule for generating C source code from LCM
%.c %.h : %.lcm 
	$(LCMGEN) $< -c

localclean::
	-$(RM) $(STRUCTS_C) $(STRUCTS_H)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
