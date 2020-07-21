## Copyright (c) 2006-2020, Universities Space Research Association (USRA).
##  All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##     * Redistributions of source code must retain the above copyright
##       notice, this list of conditions and the following disclaimer.
##     * Redistributions in binary form must reproduce the above copyright
##       notice, this list of conditions and the following disclaimer in the
##       documentation and/or other materials provided with the distribution.
##     * Neither the name of the Universities Space Research Association nor the
##       names of its contributors may be used to endorse or promote products
##       derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
## WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
## MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
## INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
## BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
## OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
## ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
## TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
## USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Expression module subproject of PLEXIL_EXEC

add_library(PlexilExpr ${Plexil_Exec_SHARED_OR_STATIC}
  Alias.cc ArithmeticOperators.cc ArrayReference.cc ArrayVariable.cc
  ArrayOperators.cc Assignable.cc BooleanOperators.cc CachedFunction.cc
  Comparisons.cc Constant.cc ConversionOperators.cc Expression.cc
  ExpressionConstants.cc Function.cc GetValueImpl.cc
  NodeConstantExpressions.cc Notifier.cc Operator.cc OperatorImpl.cc
  Propagator.cc SimpleBooleanVariable.cc StringOperators.cc UserVariable.cc)

install(TARGETS PlexilExpr
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_include_directories(PlexilExpr PRIVATE
  ${PlexilExec_SOURCE_DIR}/utils
  ${PlexilExec_SOURCE_DIR}/value)

target_link_libraries(PlexilExpr PUBLIC
  PlexilUtils PlexilValue)

if(MATH_LIBRARY)
  target_link_libraries(PlexilExpr PUBLIC ${MATH_LIBRARY})
endif()

# Public includes
install(FILES
  Alias.hh ArrayReference.hh ArrayVariable.hh Assignable.hh CachedFunction.hh
  Constant.hh ConversionOperators.hh Expression.hh ExpressionConstants.hh
  ExpressionListener.hh Function.hh GetValueImpl.hh Listenable.hh
  NodeConstantExpressions.hh Notifier.hh Operator.hh Propagator.hh
  SimpleBooleanVariable.hh UserVariable.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(MODULE_TESTS)
  add_executable(expr-module-tests
    test/aliasTest.cc test/arithmeticTest.cc test/arrayConstantTest.cc
    test/arrayOperatorsTest.cc test/arrayReferenceTest.cc
    test/arrayVariableTest.cc test/booleanOperatorsTest.cc
    test/comparisonsTest.cc test/constantsTest.cc test/conversionsTest.cc
    test/functionsTest.cc test/listenerTest.cc
    test/simpleBooleanVariableTest.cc test/stringTest.cc
    test/TrivialListener.cc test/variablesTest.cc test/expr-test-module.cc)

  install(TARGETS expr-module-tests
    DESTINATION ${CMAKE_INSTALL_BINDIR})
  
  target_include_directories(expr-module-tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PlexilExec_SOURCE_DIR}/utils
    ${PlexilExec_SOURCE_DIR}/value)
  
  target_link_libraries(expr-module-tests PlexilUtils PlexilValue PlexilExpr)

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(expr-module-tests
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()

endif()
