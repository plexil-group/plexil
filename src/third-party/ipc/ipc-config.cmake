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

# This file is NOT part of the IPC distribution.
# It is specific to the Plexil distribution.
# As such it does not attempt to be as general with respect
# to target environments as is the IPC makefile setup.
# Notably NOT supported at this writing:
#  - Languages other than C
#  - Operating systems other than macOS (Darwin) or Linux

#
# Build type default
#

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

#
# RPATH
#

# Set default RPATH for executables
if(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  set(ipc_EXE_INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}"
    CACHE STRING
    "The rpath value to use when installing IPC executables")
else()
  # Presumes system with ELF linker
  set(ipc_EXE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
    CACHE STRING
    "The rpath value to use when installing IPC executables")
endif()

#
# Target-specific settings
#

if(CMAKE_SYSTEM_NAME MATCHES Linux)

  # See etc/GNUmakefile.defs lines 329-341
  # and src/libc.h
  string(REGEX REPLACE
         [=[^([0-9][0-9]*)\..*$]=]
         [=[\1]=]
         MAJOR ${CMAKE_SYSTEM_VERSION})
  string(REGEX REPLACE
         [=[^[0-9][0-9]*\.([0-9][0-9]*).*$]=]
         [=[\1]=]
         MINOR ${CMAKE_SYSTEM_VERSION})

  if(MAJOR GREATER 2)
    # i.e. most modern Linux
    add_definitions(-DREDHAT_52 -DREDHAT_6 -DREDHAT_71)
  elseif(MAJOR EQUAL 2)
    # really ancient Linux
    add_definitions(-DREDHAT_52)
    if(MINOR GREATER_EQUAL 2)
      add_definitions(-DREDHAT_6)
      if(MINOR GREATER_EQUAL 4)
        add_definitions(-DREDHAT_71)
      endif()
    endif()

    if(MINOR LESS_EQUAL 6)
      link_libraries(bsd)
    endif()
  endif()

endif()

