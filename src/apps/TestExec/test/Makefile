
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXEC_EXE	:= ../TestExec

# dummy target for now
test-exec:

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
