# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/makefile
# Note: makefile for the <as-yet-un-named> application

agent1=sim
agent2=cmd
opts=				#-v -x -vx -d
sim_libs= -l get-state
cmd_libs= -l get-int -l get-real
cmd_line= $(agent1) $(sim_libs) $(agent2) $(cmd_libs)

include $(PLEXIL_HOME)/makeinclude/generic-plexil.make

run:
	@echo " "
	$(run_agents) $(opts) $(cmd_line)

# EOF
