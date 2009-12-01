
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	= TestExec

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml ../../interfaces/Sockets ../../interfaces/LuvListener  ../../utils ../../exec
LIB_PATH	+=
LIBS		+= tinyxml PlexilSockets LuvListener PlexilUtils PlexilExec $(PTHREAD_LIB)

SRC = ExecTestRunner.cc exec-test-runner.cc
INC = ExecTestRunner.hh

SVN_FILES       = Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
