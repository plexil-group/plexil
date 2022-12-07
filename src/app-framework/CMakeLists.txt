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

add_library(PlexilAppFramework ${PlexilExec_SHARED_OR_STATIC}
  AdapterConfiguration.cc AdapterFactory.cc CommandHandler.cc Configuration.cc
  ExecApplication.cc ExecListener.cc ExecListenerFactory.cc
  ExecListenerFilter.cc ExecListenerFilterFactory.cc ExecListenerHub.cc
  InterfaceManager.cc InterfaceSchema.cc Launcher.cc ListenerFilters.cc
  LookupHandler.cc MessageAdapter.cc SerializedInputQueue.cc SimpleInputQueue.cc
  TimeAdapter.cc Timebase.cc TimebaseFactory.cc UtilityAdapter.cc
  )

install(TARGETS PlexilAppFramework
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

include_directories(
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
  PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec
  pugixml PlexilXmlParser)

if(PlexilExec_SHLIB_INSTALL_RPATH)
  set_target_properties(PlexilAppFramework
    PROPERTIES INSTALL_RPATH ${PlexilExec_SHLIB_INSTALL_RPATH})
endif()

if(WITH_THREADS AND HAVE_LIBPTHREAD)
  target_link_libraries(PlexilAppFramework PUBLIC pthread)
endif()

if(HAVE_LIBDISPATCH)
  target_link_libraries(PlexilAppFramework PUBLIC dispatch)
endif()

if(HAVE_LIBRT)
  target_link_libraries(PlexilAppFramework PUBLIC rt)
endif()

install(FILES
  AdapterConfiguration.hh AdapterExecInterface.hh AdapterFactory.hh
  CommandHandler.hh Configuration.hh ExecApplication.hh ExecListener.hh
  ExecListenerFactory.hh ExecListenerFilter.hh ExecListenerFilterFactory.hh
  ExecListenerHub.hh InterfaceAdapter.hh InterfaceManager.hh InterfaceSchema.hh
  ListenerFilters.hh LookupHandler.hh MessageAdapter.hh PlannerUpdateHandler.hh
  SerializedInputQueue.hh SimpleInputQueue.hh Timebase.hh TimebaseFactory.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(PLAN_DEBUG_LISTENER)
  target_include_directories(PlexilAppFramework PRIVATE
    ${PlexilExec_SOURCE_DIR}/interfaces/PlanDebugListener)
  if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(PlexilAppFramework PUBLIC
      PlanDebugListener)
  endif()
endif()

if(IPC_ADAPTER)
  target_include_directories(PlexilAppFramework PRIVATE
    ${PlexilExec_SOURCE_DIR}/interfaces/IpcUtils
    ${PlexilExec_SOURCE_DIR}/interfaces/IpcAdapter)
  if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(PlexilAppFramework PUBLIC
      IpcAdapter)
  endif()
endif()

if(VIEWER_LISTENER)
  target_include_directories(PlexilAppFramework PRIVATE
    ${PlexilExec_SOURCE_DIR}/interfaces/LuvListener)
  if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(PlexilAppFramework PUBLIC
      LuvListener)
  endif()
endif()

if(UDP_ADAPTER)
  target_include_directories(PlexilAppFramework PRIVATE
    ${PlexilExec_SOURCE_DIR}/interfaces/UdpAdapter)
  if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(PlexilAppFramework PUBLIC
      UdpAdapter)
  endif()
endif()

if(MODULE_TESTS AND WITH_THREADS)
  add_executable(timebase-test
    test/timebase-test.cc Timebase.cc TimebaseFactory.cc)

  install(TARGETS timebase-test
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  target_include_directories(timebase-test PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )

  target_link_libraries(timebase-test
    PlexilUtils PlexilValue PlexilExpr PlexilIntfc PlexilExec pugixml)

  if(HAVE_LIBRT)
    target_link_libraries(timebase-test rt)
  endif()

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(timebase-test
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()

endif()
