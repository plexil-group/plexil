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

# FetchContent module requires at least this version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(PlexilExec
  DESCRIPTION "An executive for the PLEXIL language"
  LANGUAGES CXX C)

# cmake modules
INCLUDE(CMakeDependentOption)
INCLUDE(FetchContent)

#
# Options
#

#
# What to build
option(UNIVERSAL_EXEC "Build the universalExec application" ON)
option(TEST_EXEC "Build the TestExec application" ON)
option(UDP_ADAPTER "Build adapter for interfacing via UDP" OFF)
option(PLAN_DEBUG_LISTENER "Build the PlanDebugListener module" ON)
option(VIEWER_LISTENER "Build interface for Plexil Viewer" ON)
option(GANTT_LISTENER "Build interface for GANTT chart generator (deprecated)" OFF)
#
# Not strictly exec code
option(STANDALONE_SIMULATOR "Build the StandAloneSimulator application" OFF)
option(MODULE_TESTS "Build unit test executables for submodules" OFF)

#
# Implementation choices
option(POSIX_TIME "Use standard POSIX time functions" ON)
option(JAVA_NATIVE_INTERFACE "Support for calling PlexilExec from Java" OFF)
option(DEBUG_MESSAGES "Support for tracing internals at runtime" ON)

# Dependent options
CMAKE_DEPENDENT_OPTION(IPC_ADAPTER "Build TCA-IPC and adapter for interapp comms" ON
  "NOT STANDALONE_SIMULATOR" ON)
CMAKE_DEPENDENT_OPTION(POSIX_THREADS "Include POSIX threading support" ON
  "NOT IPC_ADAPTER; NOT UNIVERSAL_EXEC" ON)

#
# Create plexil-config.h from platform characteristics and options
#

include(plexil-config.cmake)

install(FILES ${CMAKE_BINARY_DIR}/plexil-config.h
  DESTINATION include)

# This is the build time location of plexil-config.h
# This is global for a good reason!
include_directories(${CMAKE_BINARY_DIR})

# TODO: create compile-time environment file


# Set default RPATH for executables on macOS
# Presumes usual structure, i.e. bin alongside lib
if(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  set(PlexilExec_EXE_INSTALL_RPATH "@executable_path/../lib"
    CACHE STRING
    "The rpath value to use when building executables")
endif()

# Include pugixml as an external project
FetchContent_Declare(
  pugixml

  # When included as separate checkout
  # GIT_REPOSITORY https://github.com/zeux/pugixml
  # GIT_TAG v1.10
  
  # When checked out as part of plexil
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/third-party/pugixml

  # Common
  PREFIX third-party/pugixml
  INSTALL_DIR ""
  )

FetchContent_GetProperties(pugixml)

if(NOT pugixml_POPULATED)
  FetchContent_Populate(pugixml
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/third-party/pugixml
    CMAKE_ARGS -DBUILD_SHARED_LIBS=ON
    )
  add_subdirectory(${pugixml_SOURCE_DIR} ${pugixml_BINARY_DIR})
endif()

# Submodules have their own shared libraries
# and unit test executables
add_subdirectory(utils)
add_subdirectory(value)
add_subdirectory(expr)
add_subdirectory(intfc)
add_subdirectory(exec)
add_subdirectory(xml-parser)
add_subdirectory(app-framework)
add_subdirectory(interfaces/Launcher)
if(PLAN_DEBUG_LISTENER)
  add_subdirectory(interfaces/PlanDebugListener)
endif()
if(VIEWER_LISTENER)
  add_subdirectory(interfaces/Sockets)
  add_subdirectory(interfaces/LuvListener)
endif()
if(TEST_EXEC)
  add_subdirectory(apps/TestExec)
endif()
if(UNIVERSAL_EXEC)
  add_subdirectory(universal-exec)
endif()
if(GANTT_LISTENER)
  add_subdirectory(interfaces/GanttListener)
endif()
if(IPC_ADAPTER)
  add_subdirectory(third-party/ipc)
  add_subdirectory(interfaces/IpcUtils)
  add_subdirectory(interfaces/IpcAdapter)
endif()
if(STANDALONE_SIMULATOR)
  add_subdirectory(apps/StandAloneSimulator)
endif()
if(UDP_ADAPTER)
  add_subdirectory(interfaces/UdpAdapter)
endif()

# Ensure entire system is built with same compiler settings

get_directory_property(subdirs DIRECTORY ${CMAKE_SOURCE_DIR} SUBDIRECTORIES)
foreach(dir ${subdirs})
  get_directory_property(dir_targets DIRECTORY ${dir} BUILDSYSTEM_TARGETS)
  list(APPEND plexil_targets ${dir_targets})
endforeach()

foreach(tgt ${plexil_targets})
  get_target_property(tgt_features ${tgt} INTERFACE_COMPILE_FEATURES)
  if(NOT ${tgt_features} MATCHES "NOTFOUND")
    list(APPEND all_compile_features ${tgt_features})
  endif()
endforeach()

list(REMOVE_DUPLICATES all_compile_features)

foreach(tgt ${plexil_targets})
  target_compile_features(${tgt} PUBLIC ${all_compile_features})
endforeach()
