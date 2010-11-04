# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/makefile
# Note: Generic makefile for plexilisp and standard plexil

# Find the .pli plans in this and the lib directory
ifneq ($(shell echo *.pli lib/*.pli),*.pli lib/*.pli)
  plans += $(shell echo *.pli lib/*.pli)
endif

# Find the .ple plans in this and lib directory
ifneq ($(shell echo *.ple lib/*.ple),*.ple lib/*.ple)
  plans += $(shell echo *.ple lib/*.ple)
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
	$(RM) *.epx *.last lib/*.epx lib/*.last

clean: dust
	$(RM) *.plx lib/*.plx

cleaner: clean
	$(RM) *~ lib/*~

# EOF
