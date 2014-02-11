#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Plexil, 3.0.0a1, plexil-support@sourceforge.net, plexil, https://plexil.wiki.sourceforge.net/)
AC_LANG(C++)
AC_CONFIG_SRCDIR([app-framework/AdapterConfiguration.cc])
AC_CONFIG_HEADER([plexil-config.h])
AC_CONFIG_MACRO_DIR([m4])

LT_INIT
AC_SUBST([LIBTOOL_DEPS])
AM_INIT_AUTOMAKE

# Configuration options
AC_ARG_WITH([jni],
        AS_HELP_STRING([--with-jni], [Include utilties for Java Native Interface (default=no)]))
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
AC_ARG_ENABLE([pointer-checks],
        AS_HELP_STRING([--enable-pointer-checks], [Check smart pointers for validity (default=yes)]))
AC_ARG_ENABLE([viewer],
        AS_HELP_STRING([--enable-viewer], [Build Plexil Viewer interface (default=yes)]))
AC_ARG_ENABLE([sas],
        AS_HELP_STRING([--enable-sas], [Build StandAloneSimulator (default=no)]))
# SAS requires IPC
AS_IF([test "x$enable_sas" = "xyes"], [
  AS_IF([test "x$enable_ipc" = "xno"], [enable_ipc=yes])
])

AC_ARG_ENABLE([test-exec],
        AS_HELP_STRING([--enable-test-exec], [Build TestExec executable (default=no)]))
AC_ARG_ENABLE([udp],
        AS_HELP_STRING([--enable-udp], [Build UdpAdapter interface (default=no)]))
AC_ARG_ENABLE([universal-exec],
        AS_HELP_STRING([--enable-universal-exec], [Build universalExec executable (default=yes)]))

# Conditionals for makefiles
# These default to on
AM_CONDITIONAL([DEBUG_LISTENER_OPT], [test "x$enable_debug_listener" != "xno"])
AM_CONDITIONAL([THREADS_OPT], [test "x$with_threads" != "xno"])
AM_CONDITIONAL([VIEWER_OPT], [test "x$enable_viewer" != "xno"])
AM_CONDITIONAL([UNIVERSAL_EXEC_OPT], [test "x$enable_universal_exec" != "xno"])

# These default to off
AM_CONDITIONAL([GANTT_OPT], [test "x$enable_gantt" = "xyes"])
AM_CONDITIONAL([IPC_OPT], [test "x$enable_ipc" = "xyes"])
AM_CONDITIONAL([JNI_OPT], [test "x$with_jni" = "xyes"])
AM_CONDITIONAL([MODULE_TESTS_OPT], [test "x$enable_module_tests" = "xyes"])
AM_CONDITIONAL([SAS_OPT], [test "x$enable_sas" = "xyes"])
AM_CONDITIONAL([TEST_EXEC_OPT], [test "x$enable_test_exec" = "xyes"])
AM_CONDITIONAL([UDP_OPT], [test "x$enable_udp" = "xyes"])

# Helpers for headers
AS_IF([test "x$enable_debug_logging" = "xno"],[
  AC_DEFINE([NO_DEBUG_MESSAGE_SUPPORT],[1],[Define to 1 if debug logging is disabled.])
])
AS_IF([test "x$enable_pointer_checks" = "xno"],[
  AC_DEFINE([PLEXIL_ID_FAST],[1],[Define to 1 if Id validity checking is disabled.])
])
AS_IF([test "x$with_threads" != "xno"],[
  AC_DEFINE([PLEXIL_WITH_THREADS],[1],[Define to 1 if multithreading is enabled.])
])

AC_DEFINE_UNQUOTED([HAVE_DEBUG_LISTENER],
  [`if test "x$enable_debug_listener" != "xno" 
    then 
      echo 1
    else
      echo 0
    fi`],
  [Define to 1 if PlanDebugListener is enabled in the build.])
AC_DEFINE_UNQUOTED([HAVE_GANTT_LISTENER], 
  [`if test "x$enable_gantt" = "xyes" 
    then 
      echo 1
    else
      echo 0
    fi`],
  [Define to 1 if GanttListener is enabled in the build.])
AC_DEFINE_UNQUOTED([HAVE_LUV_LISTENER], 
  [`if test "x$enable_viewer" != "xno" 
    then 
      echo 1
    else
      echo 0
    fi`],
  [Define to 1 if LuvListener is enabled in the build.])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
# Required by IPC
AS_IF([test "x$enable_ipc" = "xyes"], [
AC_PROG_LEX
AC_PROG_YACC
])
#AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.

AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([rt], [timer_create])
AC_CHECK_LIB([m], [sqrt])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h dlfcn.h float.h limits.h netdb.h netinet/in.h stddef.h stdint.h sys/socket.h sys/time.h unistd.h wchar.h])
# Hash table implementations
AC_CHECK_HEADERS([unordered_map tr1/unordered_map ext/hash_map backward/hash_map hash_map])

AM_CONDITIONAL([PLEXIL_DLFCN_H], [test "x$ac_cv_header_dlfcn_h" = "xyes"])

# Allow use of AC_TYPE_..._T with older 'autoconf' versions.
m4_pattern_allow([^AC_TYPE_INT32_T$])
m4_pattern_allow([^AC_TYPE_INT64_T$])
m4_pattern_allow([^AC_TYPE_UINT8_T$])
m4_pattern_allow([^AC_TYPE_UINT16_T$])
m4_pattern_allow([^AC_TYPE_UINT32_T$])
m4_pattern_allow([^AC_TYPE_UINT64_T$])
m4_pattern_allow([^AC_TYPE_SSIZE_T$])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t suseconds_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_CHECK_FUNCS([clock_gettime floor getcwd gethostbyname gettimeofday inet_ntoa localtime_r memmove memset modf pow pthread_mutexattr_settype select socket sqrt strcasecmp strchr strcspn strdup strerror strpbrk strspn strstr strtol strtoll strtoul])

AM_CONDITIONAL([PLEXIL_POSIX_TIME_FUNCS], [test "$ac_cv_func_clock_gettime" = "yes"])

# Used only in IPC
AS_IF([test "x$enable_ipc" = "xyes"], [
AC_CHECK_LIB([nsl], [inet_ntoa])
AC_CHECK_LIB([wrap], [hosts_ctl])
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h malloc.h memory.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/timeb.h termios.h])
AC_FUNC_FORK
AC_FUNC_SETPGRP
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero dup2 gethostbyaddr gethostname mkdir])
])

AC_CONFIG_FILES([Makefile
                 third-party/pugixml/src/Makefile
                 utils/Makefile
                 exec/Makefile
                 app-framework/Makefile
                 apps/TestExec/Makefile
                 interfaces/GanttListener/Makefile
                 interfaces/LuvListener/Makefile
                 interfaces/PlanDebugListener/Makefile
                 interfaces/Sockets/Makefile
                 interfaces/UdpAdapter/Makefile
                 universal-exec/Makefile])

AS_IF([test "x$enable_ipc" = "xyes"], [
AC_CONFIG_FILES([third-party/ipc-3.8.5/src/Makefile
                 interfaces/IpcAdapter/Makefile
                 interfaces/IpcUtils/Makefile
                 apps/StandAloneSimulator/Makefile])
# Add these back later?
#                 third-party/ipc-3.8.5/doc/GNUmakefile
#                 third-party/ipc-3.8.5/java/GNUmakefile
#                 third-party/ipc-3.8.5/lisp/GNUmakefile
#                 third-party/ipc-3.8.5/test/GNUmakefile
#                 third-party/ipc-3.8.5/xdrgen/GNUmakefile
])

AC_OUTPUT
