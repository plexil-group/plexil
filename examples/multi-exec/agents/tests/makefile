# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/tests/makefile
# Date: Thu Oct 14 09:46:46 2010

PLEXILISP = plexilisp

.SUFFIXES : .plx .pli

all: t1-cmd.plx t1-sim.plx t2-cmd.plx

.pli.plx :
	$(PLEXILISP) $<
	$(RM) $*.epx*

dust:
	$(RM) *.epx *.last

clean: dust
	$(RM) *.plx

cleaner: clean
	$(RM) *~

# EOF
