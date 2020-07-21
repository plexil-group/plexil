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


include(CheckCXXSourceCompiles)
include(CheckCXXSymbolExists)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)

#
# Headers
#

# Check quietly, please
set(CMAKE_REQUIRED_QUIET 1)

# Since this code may have to compile on some very backward compilers,
# we need to check for both the C++ and C versions of most C standard headers.

# C++
CHECK_INCLUDE_FILE_CXX(cassert HAVE_CASSERT)
CHECK_INCLUDE_FILE_CXX(cerrno HAVE_CERRNO)
CHECK_INCLUDE_FILE_CXX(cfloat HAVE_CFLOAT)
CHECK_INCLUDE_FILE_CXX(cinttypes HAVE_CINTTYPES)  # C++11
CHECK_INCLUDE_FILE_CXX(climits HAVE_CLIMITS)
CHECK_INCLUDE_FILE_CXX(cmath HAVE_CMATH)
CHECK_INCLUDE_FILE_CXX(csignal HAVE_CSIGNAL)
CHECK_INCLUDE_FILE_CXX(cstddef HAVE_CSTDDEF)
CHECK_INCLUDE_FILE_CXX(cstdint HAVE_CSTDINT)      # C++11
CHECK_INCLUDE_FILE_CXX(cstdio HAVE_CSTDIO)
CHECK_INCLUDE_FILE_CXX(cstdio HAVE_CSTDLIB)
CHECK_INCLUDE_FILE_CXX(cstring HAVE_CSTRING)
CHECK_INCLUDE_FILE_CXX(ctime HAVE_CTIME)

# Standard C (C90, C99)
CHECK_INCLUDE_FILE(assert.h HAVE_ASSERT_H)
CHECK_INCLUDE_FILE(errno.h HAVE_ERRNO_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS_H)
CHECK_INCLUDE_FILE(math.h HAVE_MATH_H)
CHECK_INCLUDE_FILE(signal.h HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE(stddef.h HAVE_STDDEF_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdio.h HAVE_STDIO_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)
CHECK_INCLUDE_FILE(time.h HAVE_TIME_H)

# POSIX headers
CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILE(netdb.h HAVE_NETDB_H)
CHECK_INCLUDE_FILE(pthread.h HAVE_PTHREAD_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(arpa/inet.h HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILE(netinet/in.h HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H) # GanttListener only
CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME_H)

# glibc
CHECK_INCLUDE_FILE(execinfo.h HAVE_EXECINFO_H)

# Non-Unix platformsn
CHECK_INCLUDE_FILE(vxWorks.h HAVE_VXWORKS_H)

#
# Functions
#

# Math
# These are defined as macros or intrinsics in some compilers.
#
# CHECK_FUNCTION_EXISTS fails on older g++ versions because they're macros.
# CHECK_CXX_SYMBOL_EXISTS fails on macOS clang++ because function pointer is larger than int.
# Therefore we must define our own test.

if(HAVE_CMATH OR HAVE_MATH_H)

  function(PLEXIL_CHECK_MATH_FN FNAME)
    if(HAVE_CMATH)
      set(MATH_INCLUDE cmath)
    else()
      set(MATH_INCLUDE math.h)
    endif()
    string(TOUPPER "HAVE_${FNAME}" HAVE_FNAME)
    CHECK_CXX_SOURCE_COMPILES("
#include <${MATH_INCLUDE}>

int main(int argc, char **argv)
{
  (void)argv; (void)argc;
  double temp, pi = 3.14;
  temp = ${FNAME}(pi);
  return 0;
}
"
      ${HAVE_FNAME})
  endfunction(PLEXIL_CHECK_MATH_FN)

  PLEXIL_CHECK_MATH_FN(ceil)
  PLEXIL_CHECK_MATH_FN(floor)
  PLEXIL_CHECK_MATH_FN(round)
  PLEXIL_CHECK_MATH_FN(sqrt)
  PLEXIL_CHECK_MATH_FN(trunc)
endif()

CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
CHECK_FUNCTION_EXISTS(ctime_r HAVE_CTIME_R) # utils/Logging.cc only
CHECK_FUNCTION_EXISTS(getcwd HAVE_GETCWD) # GanttListener only
CHECK_FUNCTION_EXISTS(gethostbyname HAVE_GETHOSTBYNAME) # UdpAdapter, IPC
CHECK_FUNCTION_EXISTS(getitimer HAVE_GETITIMER) # DarwinTimeAdapter, StandAloneSimulator
CHECK_FUNCTION_EXISTS(getpid HAVE_GETPID) # utils/Logging.cc, GanttListener 
CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS(isatty HAVE_ISATTY) # utils/Logging.cc only
CHECK_FUNCTION_EXISTS(localtime_r HAVE_LOCALTIME_R) # utils/test/jni-adapter.cc only
CHECK_FUNCTION_EXISTS(setitimer HAVE_SETITIMER) # DarwinTimeAdapter, StandAloneSimulator

#CHECK_CXX_SYMBOL_EXISTS(timer_create "ctime;time.h" HAVE_TIMER_CREATE)

#
# Libraries
#

find_library(MATH_LIBRARY m)

CHECK_LIBRARY_EXISTS(pthread pthread_create "/usr/lib" HAVE_LIBPTHREAD)
CHECK_LIBRARY_EXISTS(rt timer_create "/usr/lib" HAVE_LIBRT)

#
# Types
#

CHECK_TYPE_SIZE(pid_t PID_T)
CHECK_TYPE_SIZE(suseconds_t SUSECONDS_T)

#
# Compiler quirks
#

# Search on __STDC_LIMIT_MACROS, __STDC_CONSTANT_MACROS, __STDC_FORMAT_MACROS
# to understand the issues being checked for inttypes.h and stdint.h

# inttypes.h under C++
# This is a hack when compiled without including stdint.h -
# It presumes int is 32 bits
CHECK_CXX_SOURCE_COMPILES(
  "
#include <stdio.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
  printf(\"%\" PRId32 \" \", 1);
  return 0;
}
"
  HAVE_GOOD_INTTYPES_H)

# stdint.h under C++
CHECK_CXX_SOURCE_COMPILES(
  "
#include <stdint.h>

int main(int argc, char ** /* argv */)
{
  return argc < INT8_MAX;
}
"
  HAVE_GOOD_STDINT_H)

#
# For some reason timer_create and timer_delete don't show up under gcc 5.2.0
# using the normal tools.
#

set(CMAKE_REQUIRED_LIBRARIES "rt")
CHECK_CXX_SOURCE_COMPILES(
  "
#include <signal.h>
#include <time.h>

int main(int argc, char ** /* argv */)
{
  struct sigevent se; 
  timer_t tymer;

  return timer_create(CLOCK_REALTIME, &se, &tymer);
}
"
  HAVE_TIMER_CREATE)


#
# External programs
#

find_program(GPERF gperf)

#
# Build time options:
# Translate option name to internal flag
#

if(JAVA_NATIVE_INTERFACE)
  # Check JAVA_HOME
  if($ENV{JAVA_HOME})
    set(JAVA_HOME $ENV{JAVA_HOME})
  elseif(DEFINED JAVA_HOME AND JAVA_HOME)
    set(JAVA_HOME ${JAVA_HOME})
  else()
    message(FATAL_ERROR
      "JAVA_NATIVE_INTERFACE option enabled, but JAVA_HOME unset and unspecified")
  endif()
  set(HAVE_JNI ON)
else()
  unset(HAVE_JNI CACHE)
endif()

if(STANDALONE_SIMULATOR)
  set(HAVE_STANDALONE_SIM ON)
else()
  unset(HAVE_STANDALONE_SIM CACHE)
endif()

# These set corresponding macros in plexil-config.h

if(DEBUG_MESSAGES)
  unset(NO_DEBUG_MESSAGE_SUPPORT CACHE)
else()
  set(NO_DEBUG_MESSAGE_SUPPORT ON)
endif()

if(NOT POSIX_THREADS)
  unset(PLEXIL_WITH_THREADS CACHE)
elseif(NOT HAVE_LIBPTHREAD)
  message(FATAL_ERROR "POSIX_THREADS option enabled, but libpthread not found.")
else()
  set(PLEXIL_WITH_THREADS ON)
endif()

if(POSIX_TIME)
  set(PLEXIL_WITH_UNIX_TIME ON)
else()
  unset(PLEXIL_WITH_UNIX_TIME CACHE)
endif()

#
# Optional modules
#

if(GANTT_LISTENER)
  set(HAVE_GANTT_LISTENER ON)
else()
  unset(HAVE_GANTT_LISTENER CACHE)
endif()

if(IPC_ADAPTER)
  set(HAVE_IPC_ADAPTER ON)
else()
  unset(HAVE_IPC_ADAPTER CACHE)
endif()

if(PLAN_DEBUG_LISTENER)
  set(HAVE_DEBUG_LISTENER ON)
else()
  unset(HAVE_DEBUG_LISTENER CACHE)
endif()

if(VIEWER_LISTENER)
  set(HAVE_LUV_LISTENER ON)
else()
  unset(HAVE_LUV_LISTENER CACHE)
endif()

if(UDP_ADAPTER)
  set(HAVE_UDP_ADAPTER ON)
else()
  unset(HAVE_UDP_ADAPTER CACHE)
endif()
  

#
# Construct configuration header file
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/plexil-config.h.cmake.in
  ${CMAKE_BINARY_DIR}/plexil-config.h)

#
# Build type default
#

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
