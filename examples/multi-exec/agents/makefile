# -*- Mode: Makefile -*-
# File: $SVNROOT/plexil/examples/multi-exec/agents/makefile
# Date: Wed Oct  6 10:46:12 2010

plans = $(shell echo *.pli)

all: $(plans:%.pli=%.plx)

%.plx : %.pli
	plexilisp $<
	$(RM) $*.epx*

dust:
	$(RM) *.epx *.last

clean: dust
	$(RM) *.plx

cleaner: clean
	$(RM) *~

# EOF
