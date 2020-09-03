#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

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

# Because an ancient version of vxWorks only offered this version
#AC_PREREQ(2.59)
AC_PREREQ(2.62)
AC_INIT([Plexil], [4.6.0], [plexil-support@sourceforge.net], [plexil], [https://plexil.wiki.sourceforge.net/])
AC_LANG(C++)
AC_CONFIG_SRCDIR([app-framework/AdapterConfiguration.cc])
AC_CONFIG_HEADER([plexil-config.h])
AC_CONFIG_MACRO_DIR([m4])

# Check whether compilers exist and work
# Note that these two commands set CFLAGS/CXXFLAGS to '-g -O2' if not already set
AC_PROG_CXX
AC_PROG_CC

# Required to build the expression parser's table
AC_PATH_PROG([GPERF], [gperf])
AS_IF([test "x${GPERF}" = 'x'], [AC_MSG_ERROR([Required program 'gperf' not found. Aborting.])])

# Use this with autoconf 2.62 and newer
LT_INIT([])
# Use this with autoconf 2.61 and older
#AC_PROG_LIBTOOL

AC_SUBST([LIBTOOL_DEPS])

AM_INIT_AUTOMAKE

# Required with Automake versions < 1.13
#AM_PROG_CC_C_O

# Comment out this line to see the gory details of the build process
AM_SILENT_RULES([yes])

# Configuration options
AC_ARG_WITH([jni],
        AS_HELP_STRING([--with-jni=JAVA_HOME],
        [Include utilities for Java Native Interface in JAVA_HOME]),
        [AC_SUBST([JAVA_HOME], ["$with_jni"])])
AC_ARG_WITH([unix-time],
        AS_HELP_STRING([--with-unix-time], [Use Unix time facilities (default=yes)]))
AC_ARG_WITH([threads],
        AS_HELP_STRING([--with-threads], [Build for multithreaded use (default=yes)]))
AC_ARG_ENABLE([debug-listener],
        AS_HELP_STRING([--enable-debug-listener], [Build PlanDebugListener interface (default=yes)]))
AC_ARG_ENABLE([debug-logging],
        AS_HELP_STRING([--enable-debug-logging], [Allow debug output (default=yes)]))
AC_ARG_ENABLE([gantt],
        AS_HELP_STRING([--enable-gantt], [Build GanttListener interface (default=no)]))
AC_ARG_ENABLE([ipc],
        AS_HELP_STRING([--enable-ipc], [Build IPC and IpcAdapter library (default=no)]))
AC_ARG_ENABLE([module-tests],
        AS_HELP_STRING([--enable-module-tests], [Build module tests (default=no)]))
AC_ARG_ENABLE([viewer],
        AS_HELP_STRING([--enable-viewer], [Build Plexil Viewer interface (default=yes)]))
AC_ARG_ENABLE([sas],
        AS_HELP_STRING([--enable-sas], [Build StandAloneSimulator - requires IPC (default=no)]))
AC_ARG_ENABLE([test-exec],
        AS_HELP_STRING([--enable-test-exec], [Build TestExec executable (default=no)]))
AC_ARG_ENABLE([udp],
        AS_HELP_STRING([--enable-udp], [Build UdpAdapter interface (default=no)]))
AC_ARG_ENABLE([universal-exec],
        AS_HELP_STRING([--enable-universal-exec], [Build universalExec executable (default=yes)]))

# Conditionals for makefiles
# These default to enabled (with)
AM_CONDITIONAL([DEBUG_LOGGING_OPT], [test "x$enable_debug_logging" != "xno"])
AM_CONDITIONAL([DEBUG_LISTENER_OPT], [test "x$enable_debug_listener" != "xno"])
AM_CONDITIONAL([THREADS_OPT], [test "x$with_threads" != "xno"])
AM_CONDITIONAL([UNIX_TIME_OPT], [test "x$with_unix_time" != "xno"])
AM_CONDITIONAL([VIEWER_OPT], [test "x$enable_viewer" != "xno"])
AM_CONDITIONAL([UNIVERSAL_EXEC_OPT], [test "x$enable_universal_exec" != "xno"])
AM_CONDITIONAL([SHARED_LIB_OPT], [test "x$enable_shared" != "xno"])
AM_CONDITIONAL([STATIC_LIB_OPT], [test "x$enable_static" != "xno"])

# These default to disabled (without)
AM_CONDITIONAL([GANTT_OPT], [test "x$enable_gantt" = "xyes"])
AM_CONDITIONAL([IPC_OPT], [test "x$enable_ipc" = "xyes"])
AM_CONDITIONAL([JNI_OPT], [test "x$with_jni" != "x"])
AM_CONDITIONAL([MODULE_TESTS_OPT], [test "x$enable_module_tests" = "xyes"])
AM_CONDITIONAL([SAS_OPT], [test "x$enable_sas" = "xyes"])
AM_CONDITIONAL([TEST_EXEC_OPT], [test "x$enable_test_exec" = "xyes"])
AM_CONDITIONAL([UDP_OPT], [test "x$enable_udp" = "xyes"])

# Helpers for headers

# Enabled by default
AS_IF([test "x$enable_debug_logging" = "xno"],[
  AC_DEFINE([NO_DEBUG_MESSAGE_SUPPORT],[1],[Define to 1 if debug logging is disabled.])
])
AS_IF([test "x$enable_debug_listener" != "xno"],[
  AC_DEFINE([HAVE_DEBUG_LISTENER],[1],[Define to 1 if PlanDebugListener is enabled in the build.])
])
AS_IF([test "x$with_threads" != "xno"],[
  AC_DEFINE([PLEXIL_WITH_THREADS],[1],[Define to 1 if multithreading is enabled.])
])
AS_IF([test "x$with_unix_time" != "xno"],[
  AC_DEFINE([PLEXIL_WITH_UNIX_TIME],[1],[Define to 1 if Unix native time facilities are enabled.])
])
AS_IF([test "x$enable_viewer" != "xno"],[
  AC_DEFINE([HAVE_LUV_LISTENER],[1],[Define to 1 if LuvListener is enabled in the build.])
])

# Disabled by default
AS_IF([test "x$enable_gantt" = "xyes"],[
  AC_DEFINE([HAVE_GANTT_LISTENER],[1],[Define to 1 if GanttListener is enabled in the build.])
])
AS_IF([test "x$enable_ipc" = "xyes"],[
  AC_DEFINE([HAVE_IPC_ADAPTER],[1],[Define to 1 if IpcAdapter is enabled in the build.])
])
AS_IF([test "x$enable_udp" = "xyes"],[
  AC_DEFINE([HAVE_UDP_ADAPTER],[1],[Define to 1 if UdpAdapter is enabled in the build.])
])

# Checks for libraries.

# implied by header check for dlfcn.h
AC_SEARCH_LIBS([dlopen], [dl dld])
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_SEARCH_LIBS([cos], [m])
# Used by UdpAdapter (and IPC)
AC_SEARCH_LIBS([inet_ntoa], [nsl])
# POSIX timer - not present on macOS
AC_SEARCH_LIBS([timer_create], [rt])

# Checks for header files.

# C++ standard headers for PLEXIL core functionality
AC_CHECK_HEADERS_ONCE([cassert cerrno cfloat cinttypes climits cmath csignal cstddef cstdint cstdio cstdlib cstring ctime])

# Standard C (C90, C99, etc.) headers for PLEXIL core functionality
# Some platforms (e.g. vxWorks) may lack some of these
# NOTE: checks only apply to PLEXIL source code, not imported third party code
AC_CHECK_HEADERS_ONCE([assert.h errno.h float.h inttypes.h math.h stddef.h stdint.h stdio.h stdlib.h string.h time.h])
# POSIX dependencies for core functionality
AC_CHECK_HEADERS_ONCE([dlfcn.h fcntl.h pthread.h unistd.h sys/stat.h sys/time.h])
# POSIX headers for network functionality
AC_CHECK_HEADERS_ONCE([netdb.h arpa/inet.h netinet/in.h sys/socket.h])
# glibc backtrace functionality
AC_CHECK_HEADERS_ONCE([execinfo.h])

# Non-Unix-like platforms
AC_CHECK_HEADERS_ONCE([sockLib.h vxWorks.h sys/times.h])

# Check whether cstdint (stdint.h) and cinttypes (inttypes.h) behave correctly
# This is required because of a misfeature in the C99 spec

AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[@%:@include <stdint.h>]],
                   [[return argc < INT8_MAX;]])],
  [AC_DEFINE([HAVE_GOOD_STDINT_H], 1,
             [Define to 1 if stdint.h defines limit and constant macros correctly under C++])])

AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[@%:@include <stdio.h>
@%:@include <inttypes.h>]],
                   [[printf("%" PRId32 " ", 1);
                     return 0;]])],
  [AC_DEFINE([HAVE_GOOD_INTTYPES_H], 1,
  [Define to 1 if inttypes.h defines format macros correctly under C++])])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# POSIX time
# Declared in sys/time.h if present.
AC_CHECK_FUNCS([getitimer gettimeofday setitimer])
# Declared in time.h if present.
AC_CHECK_FUNCS([clock_gettime ctime_r timer_create timer_delete])
# Declared in sys/types.h, not present on (e.g.) VxWorks
# Used only in doubleToTimeval()
AC_CHECK_TYPES([suseconds_t])

# Other POSIX specifics
AC_CHECK_FUNCS([getpid isatty])

# Kernel queues (BSD family)
# Not currently used
# AC_CHECK_FUNCS([kqueue kevent kevent64])

# Standard math functions not found on some platforms
AC_CHECK_FUNCS([ceil floor round sqrt trunc])

# Used by LuvListener, UdpAdapter (and IPC)
# Obsolescent
AC_CHECK_FUNCS([gethostbyname])

# Only needed by JNI unit tests
AS_IF([test "x$with_jni" != "x"],[
# Both defined in time.h
AC_STRUCT_TM
AC_CHECK_FUNCS([localtime_r])
])

# Only needed by threads (and IPC)
AS_IF([test "x$with_threads" != "xno"],[
AC_CHECK_FUNCS([pthread_mutexattr_settype])
])

# FIXME: define PLEXIL_POSIX_TIME_FUNCS macro if appropriate functions found

# Used only in GanttListener
AS_IF([test "x$enable_gantt" = "xyes"], [
AC_CHECK_FUNCS([getcwd])
AC_CHECK_TYPES([pid_t])
])

AC_CONFIG_FILES([configure.env
                 Makefile
                 utils/Makefile
                 value/Makefile
                 expr/Makefile
                 intfc/Makefile
                 exec/Makefile
                 third-party/pugixml/src/Makefile
                 xml-parser/Makefile
                 app-framework/Makefile])

# Options that are normally on
AS_IF([test "x$enable_debug_listener" != "xno"], [
AC_CONFIG_FILES([interfaces/PlanDebugListener/Makefile])
])

AS_IF([test "x$enable_viewer" != "xno"], [
AC_CONFIG_FILES([interfaces/Sockets/Makefile
                 interfaces/LuvListener/Makefile])
])

AS_IF([test "x$enable_universal_exec" != "xno"], [
AC_CONFIG_FILES([universal-exec/Makefile])
])

# Options that are normally off
AS_IF([test "x$enable_test_exec" = "xyes"], [
AC_CONFIG_FILES([apps/TestExec/Makefile])
])

AS_IF([test "x$enable_ipc" = "xyes"], [
AC_CONFIG_FILES([third-party/ipc/Makefile
                 interfaces/IpcAdapter/Makefile
                 interfaces/IpcUtils/Makefile])
])

AS_IF([test "x$enable_sas" = "xyes"], [
AC_CONFIG_FILES([apps/StandAloneSimulator/Makefile])
])

AS_IF([test "x$enable_udp" = "xyes"], [
AC_CONFIG_FILES([interfaces/UdpAdapter/Makefile])
])

AS_IF([test "x$enable_gantt" = "xyes"], [
AC_CONFIG_FILES([interfaces/GanttListener/Makefile])
])

AC_OUTPUT
