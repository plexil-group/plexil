# Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

lib_LTLIBRARIES = libPlexilExec.la

libPlexilExec_la_CPPFLAGS = -I@top_srcdir@/third-party/pugixml/src -I@top_srcdir@/utils

include_HEADERS = Array.hh Assignment.hh AssignmentNode.hh BooleanVariable.hh Calculable.hh \
 Calculables.hh Command.hh CommandNode.hh CoreExpressions.hh ExecConnector.hh ExecDefs.hh \
 ExecListener.hh ExecListenerBase.hh ExecListenerFilter.hh ExecListenerFilterFactory.hh \
 ExecListenerHub.hh Expression.hh ExpressionFactory.hh Expressions.hh ExternalInterface.hh \
 InterfaceSchema.hh LibraryCallNode.hh ListNode.hh Lookup.hh Node.hh NodeConnector.hh \
 NodeFactory.hh PlexilExec.hh PlexilPlan.hh PlexilResource.hh PlexilXmlParser.hh \
 ResourceArbiterInterface.hh StateCache.hh TestExternalInterface.hh Update.hh UpdateNode.hh \
 Variable.hh Variables.hh plan-utils.hh resource-tags.hh

libPlexilExec_la_SOURCES = Array.cc Assignment.cc AssignmentNode.cc BooleanVariable.cc \
 Calculable.cc Calculables.cc Command.cc CommandNode.cc CoreExpressions.cc ExecListener.cc \
 ExecListenerFilter.cc ExecListenerFilterFactory.cc ExecListenerHub.cc Expression.cc \
 ExpressionFactory.cc Expressions.cc InterfaceSchema.cc LibraryCallNode.cc ListNode.cc Lookup.cc \
 Node.cc NodeFactory.cc PlexilExec.cc PlexilPlan.cc PlexilXmlParser.cc ResourceArbiterInterface.cc \
 StateCache.cc TestExternalInterface.cc Update.cc UpdateNode.cc Variable.cc Variables.cc \
 plan-utils.cc resource-tags.cc

if MODULE_TESTS_OPT
  noinst_PROGRAMS = test/exec-module-tests
  noinst_HEADERS = test/TestSupport.hh test/exec-test-module.hh
  test_exec_module_tests_SOURCES = test/exec-test-module.cc test/module-tests.cc
  test_exec_module_tests_LDADD = libPlexilExec.la @top_srcdir@/utils/libPlexilUtils.la \
   @top_srcdir@/third-party/pugixml/src/libpugixml.la
  test_exec_module_tests_CPPFLAGS = -I@top_srcdir@/third-party/pugixml/src -I@top_srcdir@/utils \
   -Itest
if JNI_OPT
    noinst_HEADERS += test/jni-adapter.hh
	test_exec_module_tests_SOURCES += test/jni-adapter.cc
endif
endif
