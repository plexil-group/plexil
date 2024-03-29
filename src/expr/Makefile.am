# Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

lib_LTLIBRARIES = libPlexilExpr.la

libPlexilExpr_la_CPPFLAGS = $(AM_CPPFLAGS) -I@top_srcdir@/value \
 -I@top_srcdir@/utils

# Public APIs
include_HEADERS = Assignable.hh Expression.hh ExpressionListener.hh \
 GetValueImpl.hh Listenable.hh NodeConnector.hh Notifier.hh

# Implementation details which don't need to be publicly advertised
noinst_HEADERS = Alias.hh ArrayReference.hh ArrayVariable.hh CachedFunction.hh \
 Constant.hh ConversionOperators.hh ExpressionConstants.hh Function.hh \
 NodeConstantExpressions.hh Operator.hh Propagator.hh Reservable.hh \
 SimpleBooleanVariable.hh UserVariable.hh

libPlexilExpr_la_SOURCES = Alias.cc \
 ArithmeticOperators.cc ArrayReference.cc ArrayVariable.cc ArrayOperators.cc \
 BooleanOperators.cc CachedFunction.cc Comparisons.cc \
 Constant.cc ConversionOperators.cc Expression.cc ExpressionConstants.cc \
 Function.cc GetValueImpl.cc NodeConstantExpressions.cc Notifier.cc \
 Operator.cc OperatorImpl.cc Propagator.cc Reservable.cc \
 SimpleBooleanVariable.cc StringOperators.cc UserVariable.cc

libPlexilExpr_la_LIBADD = @top_builddir@/value/libPlexilValue.la \
 @top_builddir@/utils/libPlexilUtils.la

if MODULE_TESTS_OPT
  bin_PROGRAMS = test/expr-module-tests
  noinst_HEADERS += test/TrivialListener.hh
  test_expr_module_tests_SOURCES = test/aliasTest.cc \
 test/arithmeticTest.cc test/arrayConstantTest.cc test/arrayOperatorsTest.cc \
 test/arrayReferenceTest.cc test/arrayVariableTest.cc \
 test/booleanOperatorsTest.cc test/comparisonsTest.cc test/constantsTest.cc \
 test/conversionsTest.cc test/functionsTest.cc test/listenerTest.cc \
 test/simpleBooleanVariableTest.cc test/stringTest.cc test/TrivialListener.cc \
 test/variablesTest.cc test/expr-test-module.cc
  test_expr_module_tests_CPPFLAGS = $(libPlexilExpr_la_CPPFLAGS)
  test_expr_module_tests_LDADD = libPlexilExpr.la $(libPlexilExpr_la_LIBADD)
endif
