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

# Standard macro definitions for Plexil make files using the ACE/TAO CORBA implementation.
# Note that these are defaults -
# overrides (if any) are defined in the platform-*.make files in this directory.


ifneq ($(MAKECMDGOALS),clean)
ifeq ($(TAO_ROOT),)
ifeq ($(ACE_ROOT),)
$(error Neither TAO_ROOT nor ACE_ROOT is set. Exiting.)
endif
TAO_ROOT := $(ACE_ROOT)/TAO
endif
endif

# Additions required by all CORBA code
INC_DIRS	+= $(ACE_ROOT) $(TAO_ROOT) $(TAO_ROOT)/orbsvcs
LIB_PATH	+= $(ACE_ROOT)/lib $(TAO_ROOT)/lib
LIBS		+= ACE TAO TAO_PortableServer TAO_AnyTypeCode TAO_CosNaming


STUBBER	:= $(TAO_ROOT)/TAO_IDL/tao_idl

# Utility definitions
idl_stub_hdrs = $(idl_file:.idl=C.h) $(idl_file:.idl=C.inl)
idl_stub_src  = $(idl_file:.idl=C.cpp)
idl_skel_hdrs = $(idl_file:.idl=S.h) $(idl_file:.idl=S.inl)
idl_skel_src  = $(idl_file:.idl=S.cpp)

# By default the TAO stubber writes its output files to the current working directory.
# Must take care to target the output where the input came from.
%C.h %C.inl %C.cpp %S.h %S.inl %S.cpp : %.idl
	 $(STUBBER) -o $(dir $<) $<
