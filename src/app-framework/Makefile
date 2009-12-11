
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= PlexilAppFramework

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml ../utils ../exec ../interfaces/Sockets
LIB_PATH	+=
LIBS		+= tinyxml PlexilUtils PlexilExec PlexilSockets $(PTHREAD_LIB)

ifneq ($(OSNAME),Darwin)
LIBS		+= rt
endif

SRC		:= $(wildcard *.cc)
INC		:= $(SRC:.cc=.hh) TimeAdapter.hh

SVN_FILES       := Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
