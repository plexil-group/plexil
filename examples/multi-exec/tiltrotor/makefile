# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/makefile
# Note: Generic makefile for plexilisp and standard plexil

# Find the .pli plans in this directory
ifneq ($(shell echo *.pli),*.pli)
  plans += $(shell echo *.pli)
endif

# Find the .ple plans in this directory
ifneq ($(shell echo *.ple),*.ple)
  plans += $(shell echo *.ple)
endif

# Find the implied .plx targets in this directory
all: $(plans:%.ple=%.plx) $(plans:%.pli=%.plx)

# Plexilisp rule
%.plx : %.pli
	plexilisp $<
	$(RM) $*.epx*

# Standard Plexil rule
%.plx : %.ple
	plexilc $<
	$(RM) $*.epx*

dust:
	$(RM) *.epx *.last

clean: dust
	$(RM) *.plx

cleaner: clean
	$(RM) *~

# EOF
