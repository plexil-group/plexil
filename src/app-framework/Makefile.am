# Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libPlexilAppFramework.la

libPlexilAppFramework_la_CPPFLAGS = $(AM_CPPFLAGS) -I@top_srcdir@/xml-parser \
 -I@top_srcdir@/third-party/pugixml/src \
 -I@top_srcdir@/exec -I@top_srcdir@/intfc -I@top_srcdir@/expr \
 -I@top_srcdir@/value -I@top_srcdir@/utils

# These additional headers are only required if the build is statically linked.
if STATIC_LIB_OPT

if DEBUG_LISTENER_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I@top_srcdir@/interfaces/PlanDebugListener
endif

if IPC_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I@top_srcdir@/interfaces/IpcAdapter
endif

if UDP_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I@top_srcdir@/interfaces/UdpAdapter
endif

if VIEWER_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I@top_srcdir@/interfaces/LuvListener
endif

endif # STATIC_LIB_OPT

include_HEADERS = AdapterConfiguration.hh AdapterExecInterface.hh \
 AdapterFactory.hh CommandHandler.hh Configuration.hh ExecApplication.hh \
 ExecListener.hh ExecListenerFactory.hh ExecListenerFilter.hh \
 ExecListenerFilterFactory.hh ExecListenerHub.hh \
 InterfaceAdapter.hh InterfaceManager.hh InterfaceSchema.hh \
 ListenerFilters.hh LookupHandler.hh MessageAdapter.hh \
 PlannerUpdateHandler.hh SerializedInputQueue.hh SimpleInputQueue.hh \
 Timebase.hh TimebaseFactory.hh

# Internal use only
noinst_HEADERS = Launcher.h TimeAdapter.h UtilityAdapter.h

libPlexilAppFramework_la_SOURCES = AdapterConfiguration.cc \
 AdapterFactory.cc CommandHandler.cc \
 Configuration.cc ExecApplication.cc ExecListener.cc ExecListenerFactory.cc \
 ExecListenerFilter.cc ExecListenerFilterFactory.cc ExecListenerHub.cc \
 InterfaceManager.cc InterfaceSchema.cc  Launcher.cc ListenerFilters.cc \
 LookupHandler.cc MessageAdapter.cc SerializedInputQueue.cc SimpleInputQueue.cc \
 TimeAdapter.cc Timebase.cc TimebaseFactory.cc UtilityAdapter.cc

# Libraries to link against
libPlexilAppFramework_la_LIBADD = @top_builddir@/xml-parser/libPlexilXmlParser.la \
 @top_builddir@/third-party/pugixml/src/libpugixml.la \
 @top_builddir@/exec/libPlexilExec.la \
 @top_builddir@/intfc/libPlexilIntfc.la \
 @top_builddir@/expr/libPlexilExpr.la \
 @top_builddir@/value/libPlexilValue.la \
 @top_builddir@/utils/libPlexilUtils.la

if MODULE_TESTS_OPT
  bin_PROGRAMS = test/timebase-test
  test_timebase_test_SOURCES = test/timebase-test.cc Timebase.cc TimebaseFactory.cc
  test_timebase_test_CPPFLAGS = $(AM_CPPFLAGS) \
   -I@top_srcdir@/third-party/pugixml/src \
   -I@top_srcdir@/intfc \
   -I@top_srcdir@/utils
  test_timebase_test_LDADD = @top_builddir@/third-party/pugixml/src/libpugixml.la \
   @top_builddir@/intfc/libPlexilIntfc.la \
   @top_builddir@/utils/libPlexilUtils.la
endif
