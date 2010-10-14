# -*- Mode: Makefile -*-
# File: $SVNROOT/plexil/examples/multi-exec/agents/tests/makefile
# Date: Tue Oct 12 16:57:52 2010

all: t1-cmd t2-cmd

t1-cmd:
	plexilisp t1-cmd.pli
	plexilisp t1-sim.pli
	make dust

t2-cmd: t1-cmd
	plexilisp t2-cmd.pli
	make dust

dust:
	$(RM) *.epx *.last
clean: dust
	$(RM) *.plx

cleaner: clean
	$(RM) *~

# EOF
