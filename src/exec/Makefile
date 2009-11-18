
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		= PlexilExec

# External dependencies
INC_DIRS	+= ../utils $(PLEXIL_HOME)/third-party/tinyxml
LIB_PATH	+=
LIBS		+= PlexilUtils tinyxml

SRC = $(wildcard *.cc)
INC = $(SRC:.cc=.hh) ExecDefs.hh PlexilResource.hh CommandHandle.hh

SVN_FILES       = Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
