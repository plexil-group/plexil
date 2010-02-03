
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	:= IpcExec

INC_DIRS	+= $(PLEXIL_HOME)/third-party/tinyxml \
		   $(PLEXIL_HOME)/third-party/ipc/src \
		   $(PLEXIL_HOME)/src/utils $(PLEXIL_HOME)/src/exec $(PLEXIL_HOME)/src/app-framework \
		   $(PLEXIL_HOME)/src/interfaces/Sockets $(PLEXIL_HOME)/src/interfaces/IpcAdapter
LIBS		+= dl ipc tinyxml $(PTHREADS_LIBRARY) PlexilUtils PlexilExec PlexilAppFramework PlexilSockets IpcAdapter

SRC		:= $(wildcard *.cc)
INC		:= $(wildcard *.hh)

SVN_FILES       := Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
