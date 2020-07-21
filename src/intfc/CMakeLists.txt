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

# Interface module subproject of PLEXIL_EXEC

add_library(PlexilIntfc ${PlexilExec_SHARED_OR_STATIC}
  CachedValue.cc CachedValueImpl.cc Command.cc CommandFunction.cc
  CommandHandleVariable.cc CommandOperator.cc CommandOperatorImpl.cc
  ExprVec.cc ExternalInterface.cc InterfaceError.cc Lookup.cc QueueEntry.cc
  ResourceArbiterInterface.cc State.cc StateCacheEntry.cc StateCacheMap.cc
  Update.cc)

install(TARGETS PlexilIntfc
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_include_directories(PlexilIntfc PRIVATE
  ${PlexilExec_SOURCE_DIR}/utils
  ${PlexilExec_SOURCE_DIR}/value
  ${PlexilExec_SOURCE_DIR}/expr)

target_link_libraries(PlexilIntfc PUBLIC
  PlexilUtils PlexilValue PlexilExpr)

# Public includes
install(FILES 
  CachedValue.hh CachedValueImpl.hh Command.hh CommandFunction.hh
  CommandHandleVariable.hh ExprVec.hh ExternalInterface.hh InputQueue.hh
  InterfaceError.hh Lookup.hh NodeConnector.hh QueueEntry.hh
  ResourceArbiterInterface.hh State.hh StateCacheEntry.hh StateCacheMap.hh
  Update.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(MODULE_TESTS)
  add_executable(intfc-module-tests
    ${PlexilExec_SOURCE_DIR}/expr/test/TrivialListener.cc
    test/lookupsTest.cc test/serializeTest.cc test/stateTest.cc
    test/intfc-test-module.cc)

  install(TARGETS intfc-module-tests
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  target_include_directories(intfc-module-tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PlexilExec_SOURCE_DIR}/utils
    ${PlexilExec_SOURCE_DIR}/value
    ${PlexilExec_SOURCE_DIR}/expr)

  target_link_libraries(intfc-module-tests PUBLIC
    PlexilUtils PlexilValue PlexilExpr PlexilIntfc)

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(intfc-module-tests
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()

endif()
