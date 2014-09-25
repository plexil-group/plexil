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

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libPlexilXmlParser.la

libPlexilXmlParser_la_CPPFLAGS = -I@top_srcdir@/third-party/pugixml/src \
 -I@top_srcdir@/utils -I@top_srcdir@/expr -I@top_srcdir@/intfc -I@top_srcdir@/exec

include_HEADERS = parsePlan.hh

libPlexilXmlParser_la_SOURCES = parseLibraryCall.cc parsePlan.cc

# *** TODO ***
# if MODULE_TESTS_OPT
#   bin_PROGRAMS = test/parser-module-tests
#   noinst_HEADERS =
#   test_parser_module_tests_SOURCES = test/parser-test-module.cc
#   test_parser_module_tests_LDADD = libPlexilXmlParser.la ../exec/libPlexilExec.la ../intfc/libPlexilIntfc.la \
#  ../expr/libPlexilExpr.la ../utils/libPlexilUtils.la \
#  ../third-party/pugixml/src/libpugixml.la
#   test_parser_module_tests_CPPFLAGS = -I@top_srcdir@/third-party/pugixml/src \
#  -I@top_srcdir@/utils -I@top_srcdir@/expr -I@top_srcdir@/intfc -I@top_srcdir@/exec
# endif
