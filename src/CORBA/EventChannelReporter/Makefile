
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY		:= PlexilEventChannelReporter

# External dependencies
INC_DIRS	+= $(ACE_ROOT) $(TAO_ROOT) $(TAO_ROOT)/orbsvcs \
		   $(PLEXIL_HOME)/third-party/tinyxml \
		   $(PLEXIL_HOME)/src/utils $(PLEXIL_HOME)/src/exec $(PLEXIL_HOME)/src/app-framework \
		   ../utils
LIB_PATH	+= $(ACE_ROOT)/lib $(PLEXIL_HOME)/lib
LIBS		+= ACE TAO TAO_PortableServer TAO_AnyTypeCode TAO_CosNaming \
		   TAO_CosEvent TAO_CosEvent_Skel TAO_CosNotification TAO_CosNotification_Skel \
		   tinyxml PlexilUtils PlexilExec PlexilAppFramework \
		   PlexilCorbaUtils

SRC		:= $(wildcard *.cc)
INC		:= $(wildcard *.hh)

SVN_FILES       := Makefile $(SRC) $(INC)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
