#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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
AC_PREREQ(2.59)
AC_INIT(Plexil, 4.5.0a1, plexil-support@sourceforge.net, plexil, https://plexil.wiki.sourceforge.net/)
AC_LANG(C++)
AC_CONFIG_SRCDIR([app-framework/AdapterConfiguration.cc])
AC_CONFIG_HEADER([plexil-config.h])
AC_CONFIG_MACRO_DIR([m4])

#LT_INIT
AC_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])
AM_INIT_AUTOMAKE

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

# These default to disabled (without)
AM_CONDITIONAL([GANTT_OPT], [test "x$enable_gantt" = "xyes"])
AM_CONDITIONAL([IPC_OPT], [test "x$enable_ipc" = "xyes"])
AM_CONDITIONAL([JNI_OPT], [test "x$with_jni" != "x"])
AM_CONDITIONAL([MODULE_TESTS_OPT], [test "x$enable_module_tests" = "xyes"])
AM_CONDITIONAL([SAS_OPT], [test "x$enable_sas" = "xyes"])
AM_CONDITIONAL([TEST_EXEC_OPT], [test "x$enable_test_exec" = "xyes"])
AM_CONDITIONAL([UDP_OPT], [test "x$enable_udp" = "xyes"])

# Helpers for headers
AS_IF([test "x$enable_debug_logging" = "xno"],[
  AC_DEFINE([NO_DEBUG_MESSAGE_SUPPORT],[1],[Define to 1 if debug logging is disabled.])
])
AS_IF([test "x$with_threads" != "xno"],[
  AC_DEFINE([PLEXIL_WITH_THREADS],[1],[Define to 1 if multithreading is enabled.])
])
AS_IF([test "x$with_unix_time" != "xno"],[
  AC_DEFINE([PLEXIL_WITH_UNIX_TIME],[1],[Define to 1 if Unix native time facilities are enabled.])
])
AS_IF([test "x$enable_debug_listener" != "xno"],[
  AC_DEFINE([HAVE_DEBUG_LISTENER],[1],[Define to 1 if PlanDebugListener is enabled in the build.])
])
# Not used?
#AS_IF([test "x$enable_gantt" = "xyes"],[
#  AC_DEFINE([HAVE_GANTT_LISTENER],[1],[Define to 1 if GanttListener is enabled in the build.])
#])
AS_IF([test "x$enable_viewer" != "xno"],[
  AC_DEFINE([HAVE_LUV_LISTENER],[1],[Define to 1 if LuvListener is enabled in the build.])
])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
#AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.

# implied by header check for dlfcn.h
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([m], [sqrt])
# POSIX timer - not present on macOS
AC_CHECK_LIB([rt], [timer_create])

# Checks for header files.

# Checks for ISO C90 (ANSI C89) header files stdlib.h, stdarg.h, string.h, float.h
AC_HEADER_STDC
# time.h and sys/time.h
AC_HEADER_TIME
AC_CHECK_HEADERS([sys/time.h])    
# Newer standard C (C95, C99, etc.) headers for PLEXIL core functionality
# Some platforms (e.g. vxWorks) may lack some of these
# NOTE: pugixml depends on wchar.h, but does not use these configuration files.
AC_CHECK_HEADERS([stdint.h])
# Unix (POSIX) dependencies for core functionality
AC_CHECK_HEADERS([dlfcn.h execinfo.h unistd.h sys/types.h])
# POSIX headers for network functionality
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h sys/socket.h])

# Conditional used in utils module Makefile.am
AM_CONDITIONAL([PLEXIL_DLFCN_H], [test "x$ac_cv_header_dlfcn_h" = "xyes"])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Dynamic library loading
AC_CHECK_FUNCS([dlopen dlclose dlerror dlsym])
# Logging
AC_CHECK_FUNCS([backtrace backtrace_symbols ctime_r])
# POSIX time
AC_CHECK_FUNCS([clock_gettime getitimer gettimeofday setitimer socket timer_create timer_delete])
AC_CHECK_TYPES([suseconds_t])
# Standard math functions not found on some platforms
AC_CHECK_FUNCS([floor modf pow sqrt])
# String parsing
AC_CHECK_FUNCS([strchr strcspn strspn strstr strtol strtoul])
# Miscellaneous
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memset])
# These types have special macros in newer autoconf versions,
# which aren't present in 2.59
AC_CHECK_TYPES([int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t])
AC_CHECK_TYPES([size_t, ssize_t])

# Used by LuvListener, UdpAdapter (and IPC)
AC_CHECK_FUNCS([gethostbyname inet_ntoa socket])

# Only needed by JNI unit tests
AS_IF([test "x$with_jni" != "x"],[
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

# Used only in IPC, which uses a separate build system
# AS_IF([test "x$enable_ipc" = "xyes"], [
# AC_PROG_GCC_TRADITIONAL
# AC_PROG_LEX
# AC_PROG_YACC
# AC_CHECK_LIB([nsl], [inet_ntoa])
# AC_CHECK_LIB([wrap], [hosts_ctl])
# AC_HEADER_SYS_WAIT
# AC_CHECK_HEADERS([fcntl.h malloc.h memory.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/timeb.h termios.h])
# AC_FUNC_FORK
# AC_FUNC_SELECT_ARGTYPES
# AC_FUNC_SETPGRP
# AC_FUNC_SETVBUF_REVERSED
# AC_FUNC_STRFTIME
# AC_FUNC_VPRINTF
# AC_CHECK_FUNCS([bzero dup2 gethostbyaddr gethostname mkdir select strdup strpbrk])
# AC_STRUCT_TM
#])

AC_CONFIG_FILES([Makefile
                 utils/Makefile
                 value/Makefile
                 expr/Makefile
                 intfc/Makefile
                 exec/Makefile
                 third-party/pugixml/src/Makefile
                 xml-parser/Makefile
                 app-framework/Makefile
                 interfaces/Launcher/Makefile])

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
