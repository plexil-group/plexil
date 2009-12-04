
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= plexilsimulator

SRC		:= $(wildcard *.cc)
INC		:= $(wildcard *.hh)

LIBS		+= PlexilUtils lcm ComStructs standalonesimulator

ifndef LCM_PATH
LCM_PATH	:= $(PLEXIL_HOME)/third-party/lcm
endif

INC_DIRS 	+= .. ../../../utils $(PLEXIL_HOME)/src/interfaces/lcm-structs $(LCM_PATH)
LIB_PATH	+= $(LCM_PATH)/lcm/.libs

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
