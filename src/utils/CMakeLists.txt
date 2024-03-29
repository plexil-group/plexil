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

# Utils module subproject of PLEXIL_EXEC

add_library(PlexilUtils ${PlexilExec_SHARED_OR_STATIC}
  DynamicLoader.cc Error.cc Logging.cc ParserException.cc PlanError.cc
  bitsetUtils.cc lifecycle-utils.c stricmp.c timespec-utils.cc timeval-utils.cc)

install(TARGETS PlexilUtils
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Public APIs
install(FILES
  Debug.hh DynamicLoader.h Error.hh Logging.hh ParserException.hh PlanError.hh
  SimpleMap.hh lifecycle-utils.h plexil-stdint.h stricmp.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(${JAVA_NATIVE_INTERFACE})
  # Additional support for Java Native Interface
  target_sources(PlexilUtils PRIVATE
    JNIHelper.cc JNIUtils.cc)
  target_include_directories(PlexilUtils PRIVATE
    ${JAVA_HOME}/include ${JAVA_HOME}/Headers)

  install(FILES
    JNIHelper.hh JNIUtils.hh ScopedOstreamRedirect.hh
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(CMAKE_DL_LIBS)
  target_link_libraries(PlexilUtils PUBLIC ${CMAKE_DL_LIBS})
endif()

if(${WITH_THREADS})
  # Additional support for multithreading
  target_sources(PlexilUtils PRIVATE
    ThreadSemaphore.cc)
  target_link_libraries(PlexilUtils PUBLIC pthread)
  install(FILES
    ThreadSemaphore.hh
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(NOT NO_DEBUG_MESSAGE_SUPPORT)
  target_sources(PlexilUtils PRIVATE DebugMessage.cc)
  install(FILES DebugMessage.hh
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(MODULE_TESTS)
  add_executable(utils-module-tests
    test/LinkedQueueTest.cc test/SimpleMapTest.cc test/SimpleSetTest.cc
    test/TestData.cc test/bitsetUtilsTest.cc test/module-tests.cc
    test/util-test-module.cc)

  install(TARGETS utils-module-tests
    DESTINATION ${CMAKE_INSTALL_BINDIR})

  target_include_directories(utils-module-tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR})

  target_link_libraries(utils-module-tests PRIVATE
    PlexilUtils)

  if(PlexilExec_EXE_INSTALL_RPATH)
    set_target_properties(utils-module-tests
      PROPERTIES INSTALL_RPATH ${PlexilExec_EXE_INSTALL_RPATH})
  endif()

  if(JAVA_NATIVE_INTERFACE)
    target_sources(utils-module-tests PRIVATE
      test/jni-adapter.cc)
    target_include_directories(utils-module-tests PRIVATE
      ${JAVA_HOME}/include ${JAVA_HOME}/Headers)
  endif()
endif()
