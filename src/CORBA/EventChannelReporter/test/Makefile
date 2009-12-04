
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

# External dependencies
INC_DIRS	+= $(ACE_ROOT) $(TAO_ROOT) $(TAO_ROOT)/orbsvcs \
		   $(PLEXIL_HOME)/third-party/tinyxml \
		   $(PLEXIL_HOME)/src/utils $(PLEXIL_HOME)/src/exec $(PLEXIL_HOME)/src/app-framework \
		   ../../utils ..
LIB_PATH	+= $(ACE_ROOT)/lib $(PLEXIL_HOME)/lib
LIBS		+= ACE TAO TAO_PortableServer TAO_AnyTypeCode TAO_CosNaming \
		   TAO_CosEvent TAO_CosEvent_Skel TAO_CosNotification TAO_CosNotification_Skel \
		   tinyxml PlexilUtils PlexilExec PlexilAppFramework \
		   PlexilCorbaUtils PlexilEventChannelReporter

SRC		:= $(wildcard *.cc)

SVN_FILES       = Makefile $(SRC)

# dummy for now
default: event-channel-test-runner notification-test-runner

event-channel-test-runner: event-channel-test-runner.o depend
	$(LD) $(LIB_PATH_FLAGS) $(LIB_FLAGS) -o $@ $<

notification-test-runner: notification-test-runner.o depend
	$(LD) $(LIB_PATH_FLAGS) $(LIB_FLAGS) -o $@ $<

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
