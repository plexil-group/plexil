
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	= TestExecSAS

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml \
		   $(PLEXIL_HOME)/third-party/lcm \
		   ../../utils ../../exec ../../app-framework \
		   ../../interfaces/lcm-structs  ../../interfaces/Sockets
LIB_PATH	+=
LIBS		+= tinyxml lcm PlexilUtils PlexilExec PlexilAppFramework PlexilSockets ComStructs

SRC = $(wildcard *.cc)
INC = $(wildcard *.hh)

SVN_FILES       = Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
