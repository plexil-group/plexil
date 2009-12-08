
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= PlexilCorbaUtils

# External dependencies
INC_DIRS	+= $(ACE_ROOT)  $(TAO_ROOT) $(TAO_ROOT)/orbsvcs $(PLEXIL_HOME)/src/utils
LIB_PATH	+= $(ACE_ROOT)/lib $(PLEXIL_HOME)/lib
LIBS		+= ACE TAO TAO_PortableServer TAO_CosNaming PlexilUtils

SRC		:= $(wildcard *.cc)
INC		:= $(SRC:.cc=.hh)

SVN_FILES       := Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
