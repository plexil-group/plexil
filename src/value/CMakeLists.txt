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

# Value representation module subproject of PLEXIL_EXEC

add_library(PlexilValue ${PlexilExec_SHARED_OR_STATIC}
  Array.cc ArrayImpl.cc CommandHandle.cc NodeConstants.cc
  PlexilTypeTraits.cc Value.cc ValueType.cc)

install(TARGETS PlexilValue
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_include_directories(PlexilValue PRIVATE
  ${PlexilExec_SOURCE_DIR}/utils)

target_link_libraries(PlexilValue PUBLIC PlexilUtils)

# Public includes
install(FILES
  Array.hh ArrayFwd.hh ArrayImpl.hh CommandHandle.hh NodeConstants.hh
  PlexilTypeTraits.hh Value.hh ValueType.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(MODULE_TESTS)
  add_executable(value-module-tests
    test/arrayTest.cc test/serializeTest.cc test/valueTest.cc
    test/valueTypeTest.cc test/value-test-module.cc)

  install(TARGETS value-module-tests
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  target_include_directories(value-module-tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PlexilExec_SOURCE_DIR}/utils)

  target_link_libraries(value-module-tests PRIVATE
    PlexilUtils PlexilValue)

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(value-module-tests
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()
endif()
