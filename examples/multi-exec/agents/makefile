# -*- Mode: Makefile -*-
# File: $SVNROOT/plexil/examples/multi-exec/agents/makefile
# Date: Wed Oct  6 10:46:12 2010

PLEXILISP = plexilisp

.SUFFIXES : .plx .pli

all: cmd.plx sim.plx

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
