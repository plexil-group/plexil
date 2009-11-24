
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	= AppFrameworkTest

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml ../../utils ../../exec .. ../../interfaces/Sockets
LIB_PATH	+=
LIBS		+= tinyxml PlexilUtils PlexilExec PlexilSockets PlexilAppFramework

SRC = testMain.cc
INC =

SVN_FILES       = Makefile $(SRC) $(INC) time-lookup-test.ple

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
