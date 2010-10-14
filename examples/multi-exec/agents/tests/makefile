# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/tests/makefile
# Date: Thu Oct 14 10:45:26 2010

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
