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

# Utils module subproject of PLEXIL_EXEC

add_library(PlexilUtils SHARED
  DebugMessage.cc Error.cc
  Logging.cc ParserException.cc PlanError.cc bitsetUtils.cc
  lifecycle-utils.c stricmp.c timespec-utils.cc timeval-utils.cc)

# Public includes
install(FILES
  ConstantMacros.hh Debug.hh DebugMessage.hh DynamicLoader.h Error.hh
  LinkedQueue.hh Logging.hh ParserException.hh PlanError.hh
  SimpleMap.hh TestSupport.hh 
  bitsetUtils.hh lifecycle-utils.h map-utils.hh stricmp.h
  timespec-utils.hh timeval-utils.hh
  DESTINATION ${PLEXIL_INCLUDE_DIR})

if(${PLEXIL_WITH_THREADS})
  # Additional support for multithreading
  target_sources(PlexilUtils PRIVATE
    RecursiveThreadMutex.cc ThreadMutex.cc ThreadSemaphore.cc ThreadSpawn.cc)
  target_link_libraries(PlexilUtils PUBLIC pthread)
  install(FILES
    RecursiveThreadMutex.hh ThreadMutex.hh ThreadSemaphore.hh ThreadSpawn.hh
    DESTINATION ${PLEXIL_INCLUDE_DIR})
endif()

if(${HAVE_LIBDL})
  # Additional support for dynamic library loading
  target_sources(PlexilUtils PRIVATE DynamicLoader.cc)
  target_link_libraries(PlexilUtils PUBLIC dl)
  install(FILES
    DynamicLoader.h
    DESTINATION ${PLEXIL_INCLUDE_DIR})
endif()

if(${WITH_JNI})
  # Additional support for Java Native Interface
  target_sources(PlexilUtils PRIVATE JNIHelper.cc JNIUtils.cc)
  target_include_directories(PlexilUtils PRIVATE ${JAVA_HOME}/include ${JAVA_HOME}/Headers)

  install(FILES
    JNIHelper.hh JNIUtils.hh ScopedOstreamRedirect.hh
    DESTINATION ${PLEXIL_INCLUDE_DIR})
endif()

if(${WITH_MODULE_TESTS})
  add_executable(utils-module-tests
    test/LinkedQueueTest.cc test/SimpleMapTest.cc test/TestData.cc
    bitsetUtilsTest.cc module-tests.cc util-test-module.cc)

  if(${WITH_JNI})
    get_target_property(utils-module-tests_SOURCES utils_module_tests SOURCES)
    list(APPEND utils-module-tests_SOURCES test/jni-adapter.cc)
    set_target_properties(utils-module-tests
      PROPERTIES SOURCES ${utils-module-tests_SOURCES})
  endif()

  install(PROGRAMS utils-module-tests
    DESTINATION ${PLEXIL_EXE_DIR})
endif()
