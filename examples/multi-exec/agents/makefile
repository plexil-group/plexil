# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/makefile
# Note: makefile for the <as-yet-un-named> application

include $(PLEXIL_HOME)/makeinclude/generic-plexil.make

AGENT1=sim
AGENT2=cmd
OPTS=				#-v -x -vx -d
SIM_LIBS=			#-l get-state
CMD_LIBS=			#-l get-int -l get-real
CMD_LINE= $(AGENT1) $(SIM_LIBS) $(AGENT2) $(CMD_LIBS)

run:
	@echo " "
	$(RUN_AGENTS) $(OPTS) $(CMD_LINE)

.PHONY: run

# EOF
