# Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
#

# Android makefile for PlexilExec module

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := PlexilExec
# doesn't seem to work
# LOCAL_SRC_FILES := $(wildcard *.cc)
LOCAL_SRC_FILES := Array.cc \
 Assignment.cc \
 AssignmentNode.cc \
 BooleanVariable.cc \
 Calculable.cc \
 Calculables.cc \
 Command.cc \
 CommandNode.cc \
 CoreExpressions.cc \
 ExecListener.cc \
 ExecListenerFilter.cc \
 ExecListenerFilterFactory.cc \
 ExecListenerHub.cc \
 Expression.cc \
 ExpressionFactory.cc \
 Expressions.cc \
 InterfaceSchema.cc \
 LibraryCallNode.cc \
 ListNode.cc \
 Lookup.cc \
 Node.cc \
 NodeFactory.cc \
 PlexilExec.cc \
 PlexilPlan.cc \
 PlexilXmlParser.cc \
 ResourceArbiterInterface.cc \
 StateCache.cc \
 TestExternalInterface.cc \
 Update.cc \
 UpdateNode.cc \
 Variable.cc \
 Variables.cc \
 plan-utils.cc \
 resource-tags.cc
LOCAL_CPP_EXTENSION := .cc
LOCAL_CPPFLAGS := 
LOCAL_EXPORT_CPPFLAGS := $(LOCAL_CPPFLAGS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../third-party/pugixml/src $(LOCAL_PATH)/../utils
#LOCAL_SHARED_LIBRARIES := pugixml PlexilUtils
LOCAL_STATIC_LIBRARIES := pugixml PlexilUtils

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

