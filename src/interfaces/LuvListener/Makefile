
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= LuvListener

# External dependencies
INC_DIRS	+= ../Sockets ../../../third-party/tinyxml ../../utils ../../exec 
LIB_PATH	+=
LIBS		+= PlexilSockets tinyxml PlexilUtils PlexilExec

SRC		:= LuvListener.cc
INC		:= LuvListener.hh

SVN_FILES       := Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
