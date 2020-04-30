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

add_library(PlexilAppFramework SHARED
  AdapterConfiguration.cc AdapterExecInterface.cc AdapterFactory.cc
  DummyAdapter.cc ExecApplication.cc ExecListener.cc ExecListenerFactory.cc
  ExecListenerFilter.cc ExecListenerFilterFactory.cc ExecListenerHub.cc
  InterfaceAdapter.cc InterfaceManager.cc ListenerFilters.cc
  SerializedInputQueue.cc SimpleInputQueue.cc UtilityAdapter.cc)

install(TARGETS PlexilAppFramework
  DESTINATION lib)

target_include_directories(PlexilAppFramework PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${PlexilExec_SOURCE_DIR}/utils
  ${PlexilExec_SOURCE_DIR}/value
  ${PlexilExec_SOURCE_DIR}/expr
  ${PlexilExec_SOURCE_DIR}/intfc
  ${PlexilExec_SOURCE_DIR}/exec
  ${PlexilExec_SOURCE_DIR}/third-party/pugixml/src
  ${PlexilExec_SOURCE_DIR}/xml-parser
  )

target_link_libraries(PlexilAppFramework PUBLIC
  PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec pugixml PlexilXmlParser)

install(FILES
  AdapterConfiguration.hh AdapterExecInterface.hh 
  AdapterFactory.hh DummyAdapter.hh ExecApplication.hh ExecListener.hh 
  ExecListenerFactory.hh ExecListenerFilter.hh ExecListenerFilterFactory.hh 
  ExecListenerHub.hh InterfaceAdapter.hh InterfaceManager.hh ListenerFilters.hh 
  PlexilListener.hh
  DESTINATION include)

if(PLAN_DEBUG_LISTENER)
  target_include_directories(PlexilAppFramework PRIVATE
    ${PlexilExec_SOURCE_DIR}/interfaces/PlanDebugListener)
endif()

if(VIEWER_LISTENER)
  target_include_directories(PlexilAppFramework PRIVATE
    ${PlexilExec_SOURCE_DIR}/interfaces/LuvListener)
endif()

if(POSIX_TIME)
  target_sources(PlexilAppFramework PRIVATE
    TimeAdapterImpl.cc PosixTimeAdapter.cc DarwinTimeAdapter.cc)

  install(FILES
    TimeAdapter.hh
    DESTINATION include)
endif()
