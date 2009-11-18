
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	= exec-module-tests

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml ../../utils ..
LIB_PATH	+=
LIBS		+= tinyxml PlexilUtils PlexilExec

SRC = exec-test-module.cc module-tests.cc
INC = exec-test-module.hh

SVN_FILES       = Makefile $(SRC) $(INC)

test-utils: $(EXECUTABLE)
	./$(EXECUTABLE)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
