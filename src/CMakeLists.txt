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

# Set location for private include files
set(PLEXIL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

# Set location for executables
set(PLEXIL_EXE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Create plexil-config.h
include(plexil-config.cmake)

include_directories(${PLEXIL_INCLUDE_DIR})

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
add_subdirectory(universal-exec)
add_subdirectory(apps/TestExec)
add_subdirectory(interfaces/Launcher)
add_subdirectory(interfaces/Sockets)
add_subdirectory(interfaces/LuvListener)
add_subdirectory(interfaces/PlanDebugListener)
