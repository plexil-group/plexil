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

# XML parser module subproject of PLEXIL_EXEC

add_library(PlexilXmlParser ${PlexilExec_SHARED_OR_STATIC}
  ArithmeticFunctionFactory.cc ArrayLiteralFactory.cc ArrayVariableFactory.cc
  ArrayVariableReferenceFactory.cc ComparisonFactory.cc 
  ConcreteExpressionFactory.cc ConversionFunctionFactory.cc commandXmlParser.cc
  createExpression.cc ExpressionFactory.cc FunctionFactory.cc
  InternalExpressionFactories.cc LookupFactory.cc NodeFunctionFactory.cc
  parseAssignment.cc parseGlobalDeclarations.cc parseLibraryCall.cc
  parseNode.cc parseNodeReference.cc parsePlan.cc parser-utils.cc
  planLibrary.cc PlexilSchema.cc SymbolTable.cc updateXmlParser.cc
  UserVariableFactory.cc)

# ExpressionMap.hh is included by createExpression.cc
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ExpressionMap.hh
  MAIN_DEPENDENCY ExpressionMap.gperf
  COMMAND ${GPERF} -m 8 --output-file=ExpressionMap.hh ${CMAKE_CURRENT_LIST_DIR}/ExpressionMap.gperf
  )

set_source_files_properties(createExpression.cc
  PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ExpressionMap.hh)

install(TARGETS PlexilXmlParser
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_include_directories(PlexilXmlParser PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PlexilExec_SOURCE_DIR}/utils
  ${PlexilExec_SOURCE_DIR}/value
  ${PlexilExec_SOURCE_DIR}/expr
  ${PlexilExec_SOURCE_DIR}/intfc
  ${PlexilExec_SOURCE_DIR}/exec
  ${pugixml_SOURCE_DIR}
  )

target_link_libraries(PlexilXmlParser PUBLIC
  PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec
  -L${pugixml_LIB_DIR} -lpugixml
  )

add_dependencies(PlexilXmlParser pugixml-build)

# Public includes
install(FILES 
  createExpression.hh ExpressionFactory.hh parseNode.hh parsePlan.hh
  parser-utils.hh planLibrary.hh PlexilSchema.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_executable(analyzePlan
  analyzePlan.cc)

install(TARGETS analyzePlan
  DESTINATION ${CMAKE_INSTALL_BINDIR})

target_include_directories(analyzePlan PRIVATE
  ${PlexilExec_SOURCE_DIR}/utils
  ${PlexilExec_SOURCE_DIR}/value
  ${PlexilExec_SOURCE_DIR}/expr
  ${PlexilExec_SOURCE_DIR}/intfc
  ${PlexilExec_SOURCE_DIR}/exec
  ${pugixml_SOURCE_DIR}
  )

target_link_libraries(analyzePlan PRIVATE
  PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec
  -L${pugixml_LIB_DIR} -lpugixml
  PlexilXmlParser)

if(PlexilExec_EXE_INSTALL_RPATH)
  set_target_properties(analyzePlan
    PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
endif()

if(MODULE_TESTS)
  add_executable(parser-module-tests
    ../
    test/FactoryTestNodeConnector.cc test/TrivialNodeConnector.cc
    test/arrayReferenceXmlParserTest.cc test/commandXmlParserTest.cc
    test/constantXmlParserTest.cc test/functionXmlParserTest.cc
    test/lookupXmlParserTest.cc test/nodeXmlParserTest.cc
    test/updateXmlParserTest.cc test/variableXmlParserTest.cc
    test/parser-test-module.cc)

  install(TARGETS parser-module-tests
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  target_include_directories(parser-module-tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PlexilExec_SOURCE_DIR}/utils
    ${PlexilExec_SOURCE_DIR}/value
    ${PlexilExec_SOURCE_DIR}/expr
    ${PlexilExec_SOURCE_DIR}/intfc
    ${PlexilExec_SOURCE_DIR}/exec
    ${pugixml_SOURCE_DIR}
    )

  target_link_libraries(parser-module-tests PRIVATE
    PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec
    -L${pugixml_LIB_DIR} -lpugixml
    PlexilXmlParser)

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(parser-module-tests
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()

  add_executable(parser-benchmark
    test/benchmark.cc)

  install(TARGETS parser-benchmark
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  target_include_directories(parser-benchmark PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PlexilExec_SOURCE_DIR}/utils
    ${PlexilExec_SOURCE_DIR}/value
    ${PlexilExec_SOURCE_DIR}/expr
    ${PlexilExec_SOURCE_DIR}/intfc
    ${PlexilExec_SOURCE_DIR}/exec
    ${pugixml_SOURCE_DIR}
    )

  target_link_libraries(parser-benchmark PRIVATE
    PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec
    -L${pugixml_LIB_DIR} -lpugixml
    PlexilXmlParser)

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(parser-benchmark
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()
endif()
