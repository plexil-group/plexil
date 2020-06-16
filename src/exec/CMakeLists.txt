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

# Executive module subproject of PLEXIL_EXEC

add_library(PlexilExec ${PlexilExec_SHARED_OR_STATIC}
  Assignment.cc AssignmentNode.cc CommandNode.cc InterfaceSchema.cc
  LibraryCallNode.cc ListNode.cc NodeImpl.cc NodeFactory.cc NodeFunction.cc
  NodeOperator.cc NodeOperatorImpl.cc NodeOperators.cc NodeTimepointValue.cc
  NodeVariableMap.cc NodeVariables.cc PlexilExec.cc PlexilNodeType.cc
  UpdateNode.cc VariableConflictSet.cc plan-utils.cc)

install(TARGETS PlexilExec
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_include_directories(PlexilExec PRIVATE
  ${PlexilExec_SOURCE_DIR}/utils
  ${PlexilExec_SOURCE_DIR}/value
  ${PlexilExec_SOURCE_DIR}/expr
  ${PlexilExec_SOURCE_DIR}/intfc)

target_link_libraries(PlexilExec PUBLIC
  PlexilUtils PlexilValue PlexilExpr PlexilIntfc)

# Public includes
install(FILES 
  Assignment.hh AssignmentNode.hh CommandNode.hh ExecListenerBase.hh
  InterfaceSchema.hh LibraryCallNode.hh ListNode.hh Node.hh NodeImpl.hh
  NodeFactory.hh NodeFunction.hh NodeOperator.hh NodeOperatorImpl.hh
  NodeOperators.hh NodeTimepointValue.hh NodeTransition.hh
  NodeVariableMap.hh NodeVariables.hh PlexilExec.hh PlexilNodeType.hh
  VariableConflictSet.hh UpdateNode.hh plan-utils.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(MODULE_TESTS)
  add_executable(exec-module-tests
    test/exec-test-module.cc test/module-tests.cc)

  install(TARGETS exec-module-tests
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  if(WITH_JNI)
    get_target_property(exec-module-tests_SOURCES exec_module_tests SOURCES)
    list(APPEND exec-module-tests_SOURCES test/jni-adapter.cc)
    set_target_properties(exec-module-tests
      PROPERTIES SOURCES ${exec-module-tests_SOURCES})
  endif()
  
  target_include_directories(exec-module-tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PlexilExec_SOURCE_DIR}/utils
    ${PlexilExec_SOURCE_DIR}/value
    ${PlexilExec_SOURCE_DIR}/expr
    ${PlexilExec_SOURCE_DIR}/intfc)

  target_link_libraries(exec-module-tests
    PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec)

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(exec-module-tests
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()

endif()
