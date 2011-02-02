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

docking:
	@echo " "
	$(RUN_AGENTS) $(OPTS) dock rover

.PHONY: run docking

# EOF
