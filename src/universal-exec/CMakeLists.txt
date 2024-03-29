## Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

# universalExec program

add_executable(universalExec
  UniversalExec.cc)

install(TARGETS universalExec
  DESTINATION ${CMAKE_INSTALL_BINDIR})

target_include_directories(universalExec PRIVATE
  ${PlexilExec_SOURCE_DIR}/utils
  ${PlexilExec_SOURCE_DIR}/value
  ${PlexilExec_SOURCE_DIR}/expr
  ${PlexilExec_SOURCE_DIR}/intfc
  ${PlexilExec_SOURCE_DIR}/exec
  ${PlexilExec_SOURCE_DIR}/third-party/pugixml/src
  ${PlexilExec_SOURCE_DIR}/xml-parser
  ${PlexilExec_SOURCE_DIR}/app-framework
  )

target_link_libraries(universalExec PRIVATE
  PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec pugixml PlexilXmlParser
  PlexilAppFramework)

# These are required whether or not the build is statically linked, if
# support for the PLEXIL Viewer is enabled.
if(VIEWER_LISTENER)
  target_include_directories(universalExec PRIVATE
    ${PlexilExec_SOURCE_DIR}/interfaces/LuvListener)
  target_link_libraries(universalExec PRIVATE
    PlexilSockets LuvListener)
endif()

if(NOT BUILD_SHARED_LIBS)
  if(PLAN_DEBUG_LISTENER)
    target_include_directories(universalExec PRIVATE
      ${PlexilExec_SOURCE_DIR}/interfaces/PlanDebugListener)
    target_link_libraries(universalExec PRIVATE
      PlanDebugListener)
  endif()

  if(IPC_ADAPTER)
    target_include_directories(universalExec PRIVATE
      ${PlexilExec_SOURCE_DIR}/interfaces/IpcAdapter)
    target_link_libraries(universalExec PRIVATE
      IpcAdapter IpcUtils ipc
      )
  endif()

  if(UDP_ADAPTER)
    target_include_directories(universalExec PRIVATE
      ${PlexilExec_SOURCE_DIR}/interfaces/UdpAdapter)
    target_link_libraries(universalExec PRIVATE
      UdpAdapter)
  endif()

endif()

if(PlexilExec_EXE_INSTALL_RPATH)
  set_target_properties(universalExec
    PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
endif()
