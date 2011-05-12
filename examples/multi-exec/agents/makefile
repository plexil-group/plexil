# -*- Mode: Makefile -*-
# File: plexil/examples/multi-exec/agents/makefile
# Note: makefile for the test multi-agent applications

include $(PLEXIL_HOME)/makeinclude/generic-plexil.make

#OPTS=				#-v -x -vx -d

cmd:
	@echo " "
	$(RUN_AGENTS) $(OPTS) sim -l state-request cmd

dock:
	@echo " "
	$(RUN_AGENTS) $(OPTS) dock -l setup-bay -l update-bay rover -l dock-rover

test2: test2.plx
	plexilexec -p test2.plx

.PHONY: cmd dock test2

# EOF
