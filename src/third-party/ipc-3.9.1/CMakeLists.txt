# Copyright (c) 2006-2020, Universities Space Research Association (USRA).
#  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Universities Space Research Association nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Library 'libipc' and executable 'central'

# This file is NOT part of the IPC distribution.
# It is specific to the Plexil distribution.
# As such it does not attempt to be as general with respect
# to target environments as is the IPC makefile setup.
# Notably NOT supported at this writing:
#  - Languages other than C
#  - Operating systems other than macOS (Darwin) or Linux

cmake_minimum_required(VERSION 3.0)

project(ipc C)

set(CMAKE_C_STANDARD 90)

include(GNUInstallDirs)

#
# Options
#

option(THREADED "IPC support for multithreading" ON)
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTS "Build tests" OFF)

#
# Configuration
#

# Platform-specific configuration
include(ipc-config.cmake)

#
# libipc
#

if(BUILD_SHARED_LIBS)
  set(ipc_SHARED_OR_STATIC SHARED)
else()
  set(ipc_SHARED_OR_STATIC STATIC)
endif()

add_library(ipc ${ipc_SHARED_OR_STATIC}
  src/ipc.c
  src/queryResponse.c
  src/marshall.c
  src/timer.c
  src/list.c
  src/behaviors.c
  src/tcModError.c
  src/datamsg.c
  src/formatters.c
  src/hash.c
  src/idtable.c
  src/key.c
  src/primFmttrs.c
  src/reg.c
  src/sendMsg.c
  src/tcaMem.c
  src/tcaRef.c
  src/comModule.c
  src/com.c
  src/globalM.c
  src/globalMUtil.c
  src/strList.c
  src/modLogging.c
  src/modVar.c
  src/resMod.c
  src/parseFmttrs.c
  src/lex.c
  src/printData.c
  )

target_include_directories(ipc PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(ipc PUBLIC
  c)

if(CMAKE_DL_LIBS)
target_link_libraries(ipc PUBLIC
  ${CMAKE_DL_LIBS})
endif()

if(THREADED)
  # TEMP DEBUG
  message("Building IPC threaded")

  target_sources(ipc PRIVATE
    src/multiThread.c)
  target_compile_definitions(ipc PUBLIC
    THREADED)
  target_link_libraries(ipc PUBLIC
    pthread)
endif()

#
# central
#

add_executable(central
  src/comServer.c
  src/dispatch.c
  src/msgTap.c
  src/recvMsg.c
  src/res.c
  src/tcerror.c
  src/logging.c
  src/globalS.c
  src/centralIO.c
  src/globalVar.c
  src/centralMain.c
  )

target_include_directories(central PRIVATE
  src)

target_link_libraries(central PRIVATE
  ipc)

set_target_properties(central
  PROPERTIES INSTALL_RPATH ${ipc_EXE_INSTALL_RPATH})

#
# Tests
#

if(BUILD_TESTS)
  # TODO
endif()

#
# Installation
#

set_target_properties(ipc PROPERTIES VERSION 3.9 SOVERSION 1)
# ??
# set_target_properties(central PROPERTIES VERSION 3.9 SOVERSION 1)

install(TARGETS ipc central
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

install(FILES
  src/ipc.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
