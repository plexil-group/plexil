# Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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
include $(PLEXIL_HOME)/makeinclude/tao-defs.make

LIBRARY		:= CorbaController

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/pugixml/src \
		   $(PLEXIL_HOME)/src/utils $(PLEXIL_HOME)/src/exec $(PLEXIL_HOME)/src/app-framework \
		   ../../utils
LIB_PATH	+= $(ACE_ROOT)/lib $(PLEXIL_HOME)/lib
LIBS		+= pugixml PlexilUtils PlexilExec PlexilAppFramework PlexilCorbaUtils

IDL_DIR		:= ../IDL
IDL_FILES	:= $(wildcard $(IDL_DIR)/*.idl)
IDL_STUB_HDRS	:= $(foreach idl_file,$(notdir $(IDL_FILES)),$(idl_stub_hdrs))
IDL_STUB_SRCS	:= $(foreach idl_file,$(notdir $(IDL_FILES)),$(idl_stub_src))
IDL_SKEL_HDRS	:= $(foreach idl_file,$(notdir $(IDL_FILES)),$(idl_skel_hdrs))
IDL_SKEL_SRCS	:= $(foreach idl_file,$(notdir $(IDL_FILES)),$(idl_skel_src))
ALL_IDL_STUBS	:= $(IDL_STUB_HDRS) $(IDL_STUB_SRCS) $(IDL_SKEL_HDRS) $(IDL_SKEL_SRCS)

SRC		:= $(wildcard *.cc) $(IDL_STUB_SRCS) $(IDL_SKEL_SRCS)
INC		:= $(wildcard *.hh) $(IDL_STUB_HDRS) $(IDL_SKEL_HDRS)

SVN_FILES       := Makefile $(SRC) $(INC)

# Local target additions and overrides

localclean::
	-$(RM) $(ALL_IDL_STUBS)

%C.h %C.inl %C.cpp %S.h %S.inl %S.cpp : $(IDL_DIR)/%.idl
	 $(STUBBER) $<

depend: $(ALL_IDL_STUBS)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
