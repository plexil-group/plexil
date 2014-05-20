include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE := robotSimulator

# External dependencies
INC_DIRS	+= $(PLEXIL_HOME)/src/utils $(PLEXIL_HOME)/include \
                   $(PLEXIL_HOME)/src/interfaces/IpcUtils \
                   $(PLEXIL_HOME)/third-party/ipc/src \
                   $(PLEXIL_HOME)/src/third-party/ooid/kashmir
LIB_PATH	+= $(OPENGL_LIB_PATH)
LIBS		+= ipc PlexilUtils IpcUtils $(PTHREAD_LIB) $(OPENGL_LIBS)
EXTRA_FLAGS	+= $(OPENGL_EXTRA_FLAGS)

SRC		:= $(wildcard *.cc)
INC		:= $(wildcard *.hh)

SVN_FILES       := Makefile $(SRC) $(INC)

# a hack until we need to get VxWorks working
DEFINES += -DUNIX

include $(PLEXIL_HOME)/makeinclude/standard-targets.make

ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
