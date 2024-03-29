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

lib_LTLIBRARIES = libPlexilValue.la

libPlexilValue_la_CPPFLAGS = $(AM_CPPFLAGS) -I@top_srcdir@/utils

# Public APIs
include_HEADERS = Array.hh ArrayImpl.hh CommandHandle.hh NodeConstants.hh \
 Value.hh ValueType.hh

# Implementation details which don't need to be publicly advertised
noinst_HEADERS = PlexilTypeTraits.hh

libPlexilValue_la_SOURCES = Array.cc ArrayImpl.cc CommandHandle.cc \
 NodeConstants.cc Value.cc ValueType.cc

libPlexilValue_la_LIBADD = @top_builddir@/utils/libPlexilUtils.la

if MODULE_TESTS_OPT
  bin_PROGRAMS = test/value-module-tests
  noinst_HEADERS +=
  test_value_module_tests_SOURCES = test/arrayTest.cc test/serializeTest.cc \
 test/valueTest.cc test/valueTypeTest.cc test/value-test-module.cc
  test_value_module_tests_CPPFLAGS = $(libPlexilValue_la_CPPFLAGS)
  test_value_module_tests_LDADD = libPlexilValue.la $(libPlexilValue_la_LIBADD)
endif
