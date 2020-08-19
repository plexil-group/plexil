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

cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(PlexilExec
  DESCRIPTION "An executive for the PLEXIL language"
  LANGUAGES CXX C)

# cmake modules
include(CMakeDependentOption)
include(ExternalProject)
include(GNUInstallDirs)

#
# Options
#

#
# How to build it
option(BUILD_SHARED_LIBS "Build shared instead of static libraries" ON)
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

# TODO: create compile-time environment file plexil-config.env

# Set default RPATH for executables
if(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  set(PlexilExec_EXE_INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}"
    CACHE STRING
    "The rpath value to use when installing Plexil executables")
else()
  # Presumes system with ELF linker
  set(PlexilExec_EXE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
    CACHE STRING
    "The rpath value to use when installing Plexil executables")
endif()

#
# pugixml as an external project
#

ExternalProject_add(pugixml

  # When included as separate checkout
  # GIT_REPOSITORY https://github.com/zeux/pugixml
  # GIT_TAG v1.10
  
  # When checked out as part of plexil
  SOURCE_DIR ${PlexilExec_SOURCE_DIR}/third-party/pugixml

  # Common
  PREFIX third-party/pugixml
  CMAKE_ARGS -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  STEP_TARGETS build
 )

ExternalProject_Get_Property(pugixml SOURCE_DIR BINARY_DIR)
set(pugixml_SOURCE_DIR ${SOURCE_DIR}/src)
set(pugixml_LIB_DIR ${BINARY_DIR})

if(BUILD_SHARED_LIBS)
  set(PlexilExec_SHARED_OR_STATIC SHARED)
else()
  set(PlexilExec_SHARED_OR_STATIC STATIC)
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

# Optional modules
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
if(UDP_ADAPTER)
  add_subdirectory(interfaces/UdpAdapter)
endif()

if(IPC_ADAPTER)
  ExternalProject_Add(
    ipc

    SOURCE_DIR ${PlexilExec_SOURCE_DIR}/third-party/ipc
    PREFIX third-party/ipc
    CMAKE_ARGS -DTHREADED=ON -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    STEP_TARGETS build
    )

  ExternalProject_Get_Property(ipc SOURCE_DIR BINARY_DIR)
  set(ipc_SOURCE_DIR ${SOURCE_DIR}/src)
  set(ipc_LIB_DIR ${BINARY_DIR})

  add_subdirectory(interfaces/IpcUtils)
  add_subdirectory(interfaces/IpcAdapter)

  if(STANDALONE_SIMULATOR)
    add_subdirectory(apps/StandAloneSimulator)
  endif()
endif()

# Ensure entire system is built with same compiler settings

get_directory_property(PlexilExec_SUBDIRS DIRECTORY ${CMAKE_SOURCE_DIR} SUBDIRECTORIES)
foreach(dir ${PlexilExec_SUBDIRS})
  get_directory_property(DIR_TARGETS DIRECTORY ${dir} BUILDSYSTEM_TARGETS)
  list(APPEND PlexilExec_TARGETS ${DIR_TARGETS})
endforeach()

foreach(TGT ${PlexilExec_TARGETS})
  get_target_property(TGT_FEATURES ${TGT} INTERFACE_COMPILE_FEATURES)
  if(NOT ${TGT_FEATURES} MATCHES "NOTFOUND")
    list(APPEND PlexilExec_ALL_COMPILE_FEATURES ${TGT_FEATURES})
  endif()
endforeach()

if(PlexilExec_ALL_COMPILE_FEATURES)
  list(REMOVE_DUPLICATES PlexilExec_ALL_COMPILE_FEATURES)

  foreach(TGT ${PlexilExec_TARGETS})
    target_compile_features(${TGT} PUBLIC ${PlexilExec_ALL_COMPILE_FEATURES})
  endforeach()
endif()
