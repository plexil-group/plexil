
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= standalonesimulator

SRC		:= $(wildcard *.cc)
INC		:= $(wildcard *.hh)

############################
# Rules to build the targets

all : default plexilsim

plexilsim : default
	$(MAKE) -C PlexilSimulator

robosim : default
	$(MAKE) -C RoboSimSimulator

###################
# Rules to clean up

localclean::
	$(MAKE) -C RoboSimSimulator clean
	$(MAKE) -C PlexilSimulator clean

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
