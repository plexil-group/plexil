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


INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckTypeSize)

#
# Headers
#

# Standard C (C90, C99)
CHECK_INCLUDE_FILE(assert.h HAVE_ASSERT_H)     # should be <cassert> in C++ code
CHECK_INCLUDE_FILE(errno.h HAVE_ERRNO_H)       # should be <cerrno> in C++ code
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)       # should be <cfloat> in C++ code
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H) # should be <cinttypes> in C++11 code
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS_H)     # should be <climits> in C++ code
CHECK_INCLUDE_FILE(math.h HAVE_MATH_H)         # should be <cmath> in C++ code
CHECK_INCLUDE_FILE(stddef.h HAVE_STDDEF_H)     # should be <cstddef> in C++ code
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)     # should be <cstdint> in C++11 code
CHECK_INCLUDE_FILE(stdio.h HAVE_STDIO_H)       # should be <cstdio> in C++ code
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)     # should be <cstdlib> in C++ code
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)     # should be <cstring> in C++ code
CHECK_INCLUDE_FILE(time.h HAVE_TIME_H)

# POSIX headers
CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILE(netdb.h HAVE_NETDB_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(arpa/inet.h HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILE(netinet/in.h HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H) # GanttListener only
CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME_H)

# glibc
CHECK_INCLUDE_FILE(execinfo.h HAVE_EXECINFO_H)

# Non-Unix platforms
CHECK_INCLUDE_FILE(vxWorks.h HAVE_VXWORKS_H)

#
# Functions
#

CHECK_FUNCTION_EXISTS(ceil HAVE_CEIL)
CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
CHECK_FUNCTION_EXISTS(ctime_r HAVE_CTIME_R) # utils/Logging.cc only
CHECK_FUNCTION_EXISTS(floor HAVE_FLOOR)
CHECK_FUNCTION_EXISTS(getcwd HAVE_GETCWD) # GanttListener only
CHECK_FUNCTION_EXISTS(gethostbyname HAVE_GETHOSTBYNAME) # UdpAdapter, IPC
CHECK_FUNCTION_EXISTS(getitimer HAVE_GETITIMER) # DarwinTimeAdapter, StandAloneSimulator
CHECK_FUNCTION_EXISTS(getpid HAVE_GETPID) # utils/Logging.cc, GanttListener 
CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS(isatty HAVE_ISATTY) # utils/Logging.cc only
CHECK_FUNCTION_EXISTS(localtime_r HAVE_LOCALTIME_R) # utils/test/jni-adapter.cc only
CHECK_FUNCTION_EXISTS(round HAVE_ROUND)
CHECK_FUNCTION_EXISTS(setitimer HAVE_SETITIMER) # DarwinTimeAdapter, StandAloneSimulator
CHECK_FUNCTION_EXISTS(sqrt HAVE_SQRT)
CHECK_FUNCTION_EXISTS(timer_create HAVE_TIMER_CREATE)
CHECK_FUNCTION_EXISTS(timer_delete HAVE_TIMER_DELETE)
CHECK_FUNCTION_EXISTS(trunc HAVE_TRUNC)

#
# Libraries
#

# Not sure about location arg - is this a list?

CHECK_LIBRARY_EXISTS(dl dlopen "/usr/lib" HAVE_LIBDL)
CHECK_LIBRARY_EXISTS(m sqrt "/usr/lib" HAVE_LIBM)
# CHECK_LIBRARY_EXISTS(nsl gethostbyname "/usr/lib" HAVE_LIBNSL) # IPC - Solaris only
CHECK_LIBRARY_EXISTS(pthread pthread_create "/usr/lib" HAVE_LIBPTHREAD)
# CHECK_LIBRARY_EXISTS(rt timer_create "/usr/lib" HAVE_LIBRT) # Solaris only

#
# Types
#

CHECK_TYPE_SIZE(pid_t PID_T)
CHECK_TYPE_SIZE(suseconds_t SUSECONDS_T)

#
# Build time options
#

# These defaulted to off

if(DEFINED WITH_JNI)
  # Check JAVA_HOME
  if($ENV{JAVA_HOME})
    set(JAVA_HOME $ENV{JAVA_HOME})
  elseif(DEFINED JAVA_HOME AND JAVA_HOME)
    set(JAVA_HOME ${JAVA_HOME})
  else()
    message(FATAL_ERROR
      "WITH_JNI specified but JAVA_HOME unset and unspecified")
  endif()
endif()

if(DEFINED WITH_MODULE_TESTS)
  set(WITH_MODULE_TESTS 1)
endif()

# These have corresponding compile-time conditionals

if(WITHOUT_DEBUG_LOGGING)
  set(NO_DEBUG_MESSAGE_SUPPORT 1)
else()
  unset(NO_DEBUG_MESSAGE_SUPPORT CACHE)
endif()

if(DEFINED WITHOUT_THREADS)
  unset(PLEXIL_WITH_THREADS CACHE)
else()
  set(PLEXIL_WITH_THREADS 1)
endif()

if(DEFINED WITHOUT_UNIX_TIME)
  unset(PLEXIL_WITH_UNIX_TIME CACHE)
else()
  set(PLEXIL_WITH_UNIX_TIME 1)
endif()

if(DEFINED WITHOUT_DEBUG_LISTENER)
  unset(HAVE_DEBUG_LISTENER CACHE)
else()
  set(HAVE_DEBUG_LISTENER 1)
endif()

if(DEFINED WITHOUT_VIEWER)
  unset(HAVE_LUV_LISTENER CACHE)
else()
  set(HAVE_LUV_LISTENER 1)
endif()

#
# Construct configuration header file
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/plexil-config.h.cmake.in
  ${PLEXIL_INCLUDE_DIR}/plexil-config.h)
