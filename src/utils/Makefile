
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= PlexilUtils

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml
LIB_PATH	+=
LIBS		+= tinyxml $(PTHREAD_LIB)

SRC		:= $(wildcard *.cc)
INC		:= $(SRC:.cc=.hh) CommonDefs.hh Id.hh TestSupport.hh

SVN_FILES       := Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
