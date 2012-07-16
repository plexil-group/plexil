include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	= exec-module-tests

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/pugixml/src ../../utils ..
LIB_PATH	+=
LIBS		+= pugixml PlexilUtils PlexilExec

SRC = exec-test-module.cc module-tests.cc
INC = exec-test-module.hh

SVN_FILES       = Makefile $(SRC) $(INC)

test-exec-module: $(EXECUTABLE)
	. $(PLEXIL_HOME)/bin/plexil-setup.sh && ./$(EXECUTABLE)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
