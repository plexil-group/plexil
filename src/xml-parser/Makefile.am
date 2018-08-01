# Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

libPlexilXmlParser_la_CPPFLAGS = $(AM_CPPFLAGS) \
 -I@top_srcdir@/third-party/pugixml/src -I@top_srcdir@/exec \
 -I@top_srcdir@/intfc -I@top_srcdir@/expr -I@top_srcdir@/value \
 -I@top_srcdir@/utils

# Publicly available header files
include_HEADERS = ExpressionFactory.hh Expressions.hh parseNode.hh parsePlan.hh \
 parser-utils.hh planLibrary.hh PlexilSchema.hh

libPlexilXmlParser_la_SOURCES = ArithmeticFunctionFactory.cc \
 ArrayLiteralFactory.cc ArrayVariableFactory.cc ArrayVariableReferenceFactory.cc \
 ConcreteExpressionFactory.cc commandXmlParser.cc ExpressionFactory.cc \
 Expressions.cc FunctionFactory.cc InternalExpressionFactories.cc \
 LookupFactory.cc parseAssignment.cc parseGlobalDeclarations.cc \
 parseLibraryCall.cc parseNode.cc parsePlan.cc parser-utils.cc planLibrary.cc \
 PlexilSchema.cc SymbolTable.cc updateXmlParser.cc UserVariableFactory.cc

DEPENDED_LIBS = libPlexilXmlParser.la \
 @top_srcdir@/third-party/pugixml/src/libpugixml.la @top_srcdir@/exec/libPlexilExec.la \
 @top_srcdir@/intfc/libPlexilIntfc.la @top_srcdir@/expr/libPlexilExpr.la \
 @top_srcdir@/value/libPlexilValue.la @top_srcdir@/utils/libPlexilUtils.la

bin_PROGRAMS = analyzePlan

analyzePlan_SOURCES = analyzePlan.cc
analyzePlan_LDADD = $(DEPENDED_LIBS)
analyzePlan_CPPFLAGS = $(libPlexilXmlParser_la_CPPFLAGS)

if MODULE_TESTS_OPT
  bin_PROGRAMS += test/parser-module-tests test/benchmark
  noinst_HEADERS =
  test_parser_module_tests_SOURCES = test/parser-test-module.cc \
   test/FactoryTestNodeConnector.cc test/TrivialNodeConnector.cc \
   test/constantXmlParserTest.cc test/variableXmlParserTest.cc \
   test/arrayReferenceXmlParserTest.cc test/functionXmlParserTest.cc \
   test/commandXmlParserTest.cc test/lookupXmlParserTest.cc \
   test/updateXmlParserTest.cc test/nodeXmlParserTest.cc
  test_parser_module_tests_LDADD = $(DEPENDED_LIBS)
  test_parser_module_tests_CPPFLAGS = $(libPlexilXmlParser_la_CPPFLAGS)

  test_benchmark_SOURCES = test/benchmark.cc
  test_benchmark_LDADD = $(DEPENDED_LIBS)
  test_benchmark_CPPFLAGS = $(libPlexilXmlParser_la_CPPFLAGS)
endif
