# -*- Mode: Makefile -*-
# File: $SVNROOT/plexil/examples/multi-exec/agents/makefile
# Date: Wed Oct  6 10:46:12 2010
# Note:

all:
	plexilisp cmd.pli
	plexilisp sim.pli
	make dust

dust:
	$(RM) *.epx *.last
clean: dust
	$(RM) *.plx

cleaner: clean
	$(RM) *~

# EOF
