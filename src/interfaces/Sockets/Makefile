
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		= PlexilSockets

# External dependencies
INC_DIRS	=
LIB_PATH	=
LIBS		=

SRC = $(wildcard *.cpp)
INC = $(SRC:.cpp=.h) SocketException.h

SVN_FILES       = Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
