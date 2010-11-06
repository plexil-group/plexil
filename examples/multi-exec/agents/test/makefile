# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/makefile
# Note: Generic makefile for plexilisp and standard plexil

include $(PLEXIL_HOME)/makeinclude/generic-plexil.make
opts=

t1:
	$(run_agents) $(opts) t1-sim t1-cmd

# intermittently errors out
t2:
	$(run_agents) $(opts) t1-sim t2-cmd

# t3 hangs (but should not)
t3:
	$(run_agents) $(opts) t1-sim t3-cmd

# t4 get a run-time error and hangs (but should not)
t4:
	$(run_agents) $(opts) t4-sim t1-cmd

# EOF
