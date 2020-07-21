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

# Sockets submodule of PlexilExec project

add_library(PlexilSockets ${PlexilExec_SHARED_OR_STATIC}
  ClientSocket.cpp ServerSocket.cpp Socket.cpp)

install(TARGETS PlexilSockets
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(${WITH_MODULE_TESTS})
  add_executable(simple_client_main
    test/simple_client_main.cpp)
  target_include_directories(simple_client_main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR})
  target_link_libraries(simple_client_main PRIVATE
    PlexilSockets)

  add_executable(simple_server_main
    test/simple_client_main.cpp)
  target_include_directories(simple_server_main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR})
  target_link_libraries(simple_server_main PRIVATE
    PlexilSockets)

  install(TARGETS simple_client_main simple_server_main
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(simple_client_main simple_server_main
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()
endif()
