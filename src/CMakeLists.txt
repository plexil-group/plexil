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


# fix this later as required
cmake_minimum_required(VERSION 3.0.0)

project(PlexilExec CXX C)

# Create plexil-config.h from CMake command line options
include(plexil-config.cmake)

install(FILES ${CMAKE_BINARY_DIR}/plexil-config.h
  DESTINATION include)

# This is where plexil-config.h is written during the build
include_directories(${CMAKE_BINARY_DIR})

# Set default RPATH for executables
# Presumes usual structure, i.e. bin alongside lib
if(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  set(PlexilExec_EXE_INSTALL_RPATH "@executable_path/../lib"
    CACHE STRING
    "The rpath value to use when building executables")
endif()

# Submodules have their own shared libraries
# and unit test executables
add_subdirectory(utils)
add_subdirectory(value)
add_subdirectory(expr)
add_subdirectory(intfc)
add_subdirectory(exec)
add_subdirectory(third-party/pugixml/src)
add_subdirectory(xml-parser)
add_subdirectory(app-framework)
add_subdirectory(interfaces/Launcher)
if(DEFINED HAVE_DEBUG_LISTENER)
  add_subdirectory(interfaces/PlanDebugListener)
endif()
if(DEFINED HAVE_LUV_LISTENER)
  add_subdirectory(interfaces/Sockets)
  add_subdirectory(interfaces/LuvListener)
endif()
if(DEFINED HAVE_TEST_EXEC)
  add_subdirectory(apps/TestExec)
endif()
if(DEFINED HAVE_UNIVERSAL_EXEC)
  add_subdirectory(universal-exec)
endif()
if(DEFINED HAVE_GANTT_LISTENER)
  add_subdirectory(interfaces/GanttListener)
endif()
if(DEFINED HAVE_IPC)
  add_subdirectory(third-party/ipc)
  add_subdirectory(interfaces/IpcUtils)
  add_subdirectory(interfaces/IpcAdapter)
endif()
if(DEFINED HAVE_STANDALONE_SIM)
  add_subdirectory(apps/StandAloneSimulator)
endif()
if(DEFINED HAVE_UDP)
  add_subdirectory(interfaces/UdpAdapter)
endif()
