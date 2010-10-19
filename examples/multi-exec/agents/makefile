# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/makefile

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
