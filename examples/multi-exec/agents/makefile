# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/makefile
# Note: makefile for the test multi-agent application

include $(PLEXIL_HOME)/makeinclude/generic-plexil.make

AGENT1=sim
AGENT2=cmd
AGENT1_LIBS= -l state-request
AGENT2_LIBS=
OPTS=				#-v -x -vx -d
CMD_LINE= $(AGENT1) $(AGENT1_LIBS) $(AGENT2) $(AGENT2_LIBS)

run:
	@echo " "
	$(RUN_AGENTS) $(OPTS) $(CMD_LINE)

dock:
	@echo " "
	$(RUN_AGENTS) $(OPTS) dock -l setup-bay -l update-bay rover -l dock-rover

test2: test2.plx
	plexilexec -p test2.plx

.PHONY: run dock

# EOF
