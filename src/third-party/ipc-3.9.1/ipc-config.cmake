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
# Include file checks
#

include(CheckIncludeFile)

function(file_exists_define FNAME SYM)
  check_include_file(${FNAME} ${SYM})
  if(${SYM})
    add_definitions("-D${SYM}=1")
  endif()
endfunction()

# These are actually checked
file_exists_define(bsd/bsd.h HAVE_BSD_BSD_H)
file_exists_define(bsd/sgtty.h HAVE_BSD_SGTTY_H)
file_exists_define(ioctls.h HAVE_IOCTLS_H)
file_exists_define(sys/filio.h HAVE_SYS_FILIO_H)
file_exists_define(sys/socketvar.h HAVE_SYS_SOCKETVAR_H)
file_exists_define(sys/stream.h HAVE_SYS_STREAM_H)
file_exists_define(sys/time.h HAVE_SYS_TIME_H)
file_exists_define(sys/ttold.h HAVE_SYS_TTOLD_H)
file_exists_define(sys/ttydev.h HAVE_SYS_TTYDEV_H)
