
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

ifndef ROBOSIM_HOME
ROBOSIM_HOME = ../RobotSimulator
endif

OSTYPE   ?= $(shell uname -s)
MACHTYPE ?= $(shell uname -p)
OS_ARCH = $(OSTYPE)-$(MACHTYPE)

EXECUTABLE = roboSimExec

INC_DIRS += $(PLEXIL_HOME)/third-party/tinyxml $(PLEXIL_HOME)/src/utils $(PLEXIL_HOME)/src/exec $(PLEXIL_HOME)/src/interfaces/Sockets  $(PLEXIL_HOME)/src/app-framework $(ROBOSIM_HOME)/SSWGComm
LIBS		+= pthread tinyxml PlexilUtils PlexilExec PlexilAppFramework PlexilSockets sswgcom
LIB_DIRS	+= $(ROBOSIM_HOME)/SSWGComm/$(OS_ARCH)/lib

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
ROBOSIMEXEC_TARGET = roboSimExec

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
