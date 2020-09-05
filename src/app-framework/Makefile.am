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

lib_LTLIBRARIES = libPlexilAppFramework.la

libPlexilAppFramework_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/xml-parser \
 -I$(top_srcdir)/third-party/pugixml/src \
 -I$(top_srcdir)/exec -I$(top_srcdir)/intfc -I$(top_srcdir)/expr \
 -I$(top_srcdir)/value -I$(top_srcdir)/utils

if DEBUG_LISTENER_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I$(top_srcdir)/interfaces/PlanDebugListener
endif

if GANTT_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I$(top_srcdir)/interfaces/GanttListener
endif

if IPC_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I$(top_srcdir)/interfaces/IpcUtils \
 -I$(top_srcdir)/interfaces/IpcAdapter
endif

if UDP_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I$(top_srcdir)/interfaces/UdpAdapter
endif

if VIEWER_OPT
  libPlexilAppFramework_la_CPPFLAGS += -I$(top_srcdir)/interfaces/LuvListener
endif

# Headers that need to be seen by interface developers
include_HEADERS = commandHandlerDefs.hh lookupHandlerDefs.hh \
 plannerUpdateHandlerDefs.hh AdapterConfiguration.hh AdapterExecInterface.hh \
 AdapterFactory.hh ExecApplication.hh ExecListener.hh ExecListenerFactory.hh \
 ExecListenerFilter.hh ExecListenerFilterFactory.hh ExecListenerHub.hh \
 InterfaceAdapter.hh ListenerFilters.hh \
 PlexilListener.hh SerializedInputQueue.hh SimpleInputQueue.hh

# Headers of internal interest only
noinst_HEADERS = InterfaceManager.hh Launcher.hh UtilityAdapter.hh

libPlexilAppFramework_la_SOURCES = AdapterConfiguration.cc \
 AdapterExecInterface.cc AdapterFactory.cc ExecApplication.cc \
 ExecListener.cc ExecListenerFactory.cc ExecListenerFilter.cc \
 ExecListenerFilterFactory.cc ExecListenerHub.cc InterfaceAdapter.cc \
 InterfaceManager.cc Launcher.cc ListenerFilters.cc SerializedInputQueue.cc \
 SimpleInputQueue.cc UtilityAdapter.cc

if UNIX_TIME_OPT
include_HEADERS += TimeAdapter.hh
libPlexilAppFramework_la_SOURCES += TimeAdapterImpl.cc PosixTimeAdapter.cc \
 DarwinTimeAdapter.cc
endif

# Libraries to link against
libPlexilAppFramework_la_LIBADD = $(top_builddir)/xml-parser/libPlexilXmlParser.la \
 $(top_builddir)/third-party/pugixml/src/libpugixml.la $(top_builddir)/exec/libPlexilExec.la \
 $(top_builddir)/intfc/libPlexilIntfc.la $(top_builddir)/expr/libPlexilExpr.la \
 $(top_builddir)/value/libPlexilValue.la $(top_builddir)/utils/libPlexilUtils.la

