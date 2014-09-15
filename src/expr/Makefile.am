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

lib_LTLIBRARIES = libPlexilExpr.la

libPlexilExpr_la_CPPFLAGS = -I@top_srcdir@/utils \
 -I@top_srcdir@/third-party/pugixml/src

include_HEADERS = Alias.hh Array.hh ArrayImpl.hh \
 ArrayReference.hh ArrayVariable.hh Assignable.hh AssignableImpl.hh \
 CommandHandle.hh ConcreteExpressionFactory.hh Constant.hh Expression.hh ExpressionConstants.hh \
 ExpressionFactories.hh ExpressionFactory.hh ExpressionImpl.hh \
 ExpressionListener.hh ExprVec.hh Function.hh FunctionFactory.hh \
 NodeConnector.hh NodeConstants.hh NotifierImpl.hh Operator.hh UserVariable.hh Value.hh \
 ValueType.hh expression-schema.hh

libPlexilExpr_la_SOURCES = Alias.cc Array.cc ArrayImpl.cc \
 ArithmeticFunctionFactory.cc ArithmeticOperators.cc ArrayOperators.cc \
 ArrayReference.cc ArrayVariable.cc ArrayVariableFactory.cc \
 ArrayVariableReferenceFactory.cc Assignable.cc \
 AssignableImpl.cc BooleanOperators.cc CommandHandle.cc Comparisons.cc \
 ConcreteExpressionFactory.cc Constant.cc Expression.cc ExpressionConstants.cc \
 ExpressionFactories.cc ExpressionFactory.cc ExpressionImpl.cc \
 ExpressionListener.cc ExprVec.cc Function.cc FunctionFactory.cc \
 NodeConstants.cc NodeConstantExpressions.cc NotifierImpl.cc OperatorImpl.cc PlexilExpr.cc StringOperators.cc \
 UserVariable.cc UserVariableFactory.cc Value.cc ValueType.cc expression-schema.cc

if MODULE_TESTS_OPT
  bin_PROGRAMS = test/expr-module-tests
  noinst_HEADERS = test/TrivialListener.hh test/TrivialNodeConnector.hh \
 test/FactoryTestNodeConnector.hh
  test_expr_module_tests_SOURCES = test/aliasTest.cc test/arithmeticTest.cc \
 test/arrayConstantTest.cc test/arrayOperatorsTest.cc \
 test/arrayReferenceFactoryTest.cc test/arrayReferenceTest.cc test/arrayTest.cc \
 test/arrayVariableTest.cc test/booleanOperatorsTest.cc test/comparisonsTest.cc \
 test/constantFactoryTest.cc test/constantsTest.cc \
 test/FactoryTestNodeConnector.cc test/functionFactoryTest.cc \
 test/functionsTest.cc test/listenerTest.cc \
 test/stringTest.cc test/TrivialListener.cc test/TrivialNodeConnector.cc \
 test/valueTest.cc test/variableFactoryTest.cc test/variablesTest.cc \
 test/expr-test-module.cc
  test_expr_module_tests_CPPFLAGS = -I../third-party/pugixml/src -I../utils
  test_expr_module_tests_LDADD = libPlexilExpr.la  ../utils/libPlexilUtils.la \
 ../third-party/pugixml/src/libpugixml.la
endif
