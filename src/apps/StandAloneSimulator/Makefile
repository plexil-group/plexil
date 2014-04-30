include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= standalonesimulator

SRC		:= $(wildcard *.cc)
INC		:= $(wildcard *.hh)

INC_DIRS	+= $(PLEXIL_HOME)/src/utils $(PLEXIL_HOME)/include \
                   $(PLEXIL_HOME)/third-party/ipc/src \
		   $(PLEXIL_HOME)/src/exec \
                   $(PLEXIL_HOME)/src/interfaces/IpcUtils
                   

LIBS		+= PlexilUtils ipc IpcUtils

############################
# Rules to build the targets

all : plexil-default plexilsim

plexilsim : plexil-default
	$(MAKE) -C PlexilSimulator

# Moved to plexil-3/examples/apps/robosim
#robosim : plexil-default
#	$(MAKE) -C RoboSimSimulator

###################
# Rules to clean up

localclean::
	$(MAKE) -C RoboSimSimulator clean
	$(MAKE) -C PlexilSimulator clean

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
