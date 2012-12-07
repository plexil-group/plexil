# Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

bin_PROGRAMS = central
noinst_HEADERS =
central_SOURCES = comServer.c dispatch.c msgTap.c recvMsg.c res.c tcerror.c logging.c globalS.c \
 centralIO.c globalVar.c centralMain.c
central_LDADD = libipc.la

lib_LTLIBRARIES = libipc.la
include_HEADERS = ipc.h
libipc_la_SOURCES = ipc.c queryResponse.c marshall.c timer.c list.c behaviors.c tcModError.c \
 datamsg.c formatters.c hash.c idtable.c key.c primFmttrs.c reg.c sendMsg.c tcaMem.c tcaRef.c \
 comModule.c com.c globalM.c globalMUtil.c strList.c modLogging.c modVar.c resMod.c \
 parseFmttrs.c lex.c printData.c multiThread.c

