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

# UDP interface module for PlexilExec

add_library(UdpUtils ${PlexilExec_SHARED_OR_STATIC}
  udp-utils.cc)

target_include_directories(UdpUtils PUBLIC
  ${PlexilExec_SOURCE_DIR}/utils)

# No link dependencies here
# target_link_libraries(UdpUtils PUBLIC
#   )

install(FILES 
  udp-utils.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(MODULE_TESTS)
  add_executable(udp-tests
    test/udp-tests.cc)

  target_include_directories(udp-tests PRIVATE
    ${PlexilExec_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_LIST_DIR}
    )

  target_link_libraries(udp-tests PRIVATE
    UdpUtils PlexilUtils)

  install(TARGETS udp-tests
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(udp-tests
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()
endif()

add_library(UdpAdapter ${PlexilExec_SHARED_OR_STATIC}
  MessageQueueMap.cc UdpAdapter.cc)

target_include_directories(UdpAdapter PUBLIC
  ${PlexilExec_SOURCE_DIR}/utils
  ${PlexilExec_SOURCE_DIR}/value
  ${PlexilExec_SOURCE_DIR}/expr
  ${PlexilExec_SOURCE_DIR}/intfc
  ${PlexilExec_SOURCE_DIR}/exec
  ${PlexilExec_SOURCE_DIR}/app-framework
  ${PlexilExec_SOURCE_DIR}/third-party/pugixml/src
  )

# Direct dependencies only
target_link_libraries(UdpAdapter PUBLIC
  PlexilUtils
  PlexilValue
  PlexilIntfc
  PlexilAppFramework
  UdpUtils
  )

install(TARGETS UdpUtils UdpAdapter
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES 
  MessageQueueMap.hh UdpAdapter.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
