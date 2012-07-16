include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= PlexilExec

# External dependencies
INC_DIRS	+= ../utils $(PLEXIL_HOME)/third-party/pugixml/src
LIB_PATH	+=
LIBS		+= PlexilUtils pugixml

SRC		:= $(wildcard *.cc)
INC		:= $(SRC:.cc=.hh) ExecConnector.hh ExecDefs.hh ExternalInterface.hh NodeConnector.hh PlexilResource.hh

SVN_FILES       := Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
