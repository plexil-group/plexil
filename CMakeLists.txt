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

cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

# explicitly set certain policies
cmake_policy(VERSION 3.6)

project(Plexil
  DESCRIPTION "The PLEXIL Executive package"
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
#
# Not strictly exec code
option(STANDALONE_SIMULATOR "Build the StandAloneSimulator application" OFF)
option(MODULE_TESTS "Build unit test executables for submodules" OFF)

#
# Implementation choices
option(JAVA_NATIVE_INTERFACE "Support for calling PlexilExec from Java" OFF)
option(DEBUG_MESSAGES "Support for tracing internals at runtime" ON)

# Dependent options
CMAKE_DEPENDENT_OPTION(IPC_ADAPTER "Build TCA-IPC and adapter for interapp comms" ON
  "NOT STANDALONE_SIMULATOR" ON)
CMAKE_DEPENDENT_OPTION(WITH_THREADS "Include multithreading support" ON
  "NOT IPC_ADAPTER; NOT UNIVERSAL_EXEC" ON)

# If source directory is a git clone, fetch submodules.
# Derived from https://cliutils.gitlab.io/modern-cmake/chapters/projects/submodule.html
if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
  find_package(Git QUIET)
  if(GIT_FOUND)
    # Update submodules as needed
    option(GIT_SUBMODULE "Update submodules during build" ON)
    if(GIT_SUBMODULE)
      message(STATUS "Submodule update")
      execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT)
      if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "Fetching submodules failed with ${GIT_SUBMOD_RESULT}")
      endif()
    endif()
  else()
    message(WARNING "'git' command not found. Skipping submodule update.")
  endif()
endif()

# PlexilExec subproject
add_subdirectory(src)
