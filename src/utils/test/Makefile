
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	= utils-module-tests

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml ..
LIB_PATH	+=
LIBS		+= tinyxml PlexilUtils

SRC = util-test-module.cc module-tests.cc
INC = util-test-module.hh

SVN_FILES       = Makefile $(SRC) $(INC)

test-utils: $(EXECUTABLE)
	./$(EXECUTABLE)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
