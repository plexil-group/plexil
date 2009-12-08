
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	:= TestExecSAS

# External dependencies
ifndef LCM_PATH
LCM_PATH	:= $(PLEXIL_HOME)/third-party/lcm
endif

INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml \
		   ../../utils ../../exec ../../app-framework \
		   ../../interfaces/lcm-structs  ../../interfaces/Sockets \
		   $(LCM_PATH)
LIB_PATH	+= $(LCM_PATH)/lcm/.libs
LIBS		+= tinyxml lcm PlexilUtils PlexilExec PlexilAppFramework PlexilSockets ComStructs

SRC		:= $(wildcard *.cc)
INC		:= $(wildcard *.hh)

SVN_FILES       := Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
