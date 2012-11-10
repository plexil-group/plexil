# *** TEMPORARY ***


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

