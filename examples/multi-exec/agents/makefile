# -*- Mode: Makefile -*-
# File: $SVNROOT/examples/multi-exec/agents/makefile
# Note: Generic makefile for plexilisp and standard plexil

# Find the .pli plans in this directory
ifneq ($(shell echo *.pli), *.pli)
  plans += $(shell echo *.pli)
endif
# Find the .pli files in the lib subdirectory
ifneq ($(shell echo lib/*.pli), lib/*.pli)
  plans += $(shell echo lib/*.pli)
endif

# Find the .ple plans in this directory
ifneq ($(shell echo *.ple), *.ple)
  plans += $(shell echo *.ple)
endif
# Find the .ple plans in the lib subdirectory
ifneq ($(shell echo lib/*.ple), lib/*.ple)
  plans += $(shell echo lib/*.ple)
endif

# Find the implied .plx targets in this directory
all: $(plans:%.ple=%.plx) $(plans:%.pli=%.plx)

# Plexilisp rule
%.plx : %.pli
	plexilisp $<
	xmllint --noout --schema $(PLEXIL_HOME)/schema/core-plexil.xsd $*.plx
	$(RM) $*.epx*

# Standard Plexil rule
%.plx : %.ple
	plexilc $<
	xmllint --noout --schema $(PLEXIL_HOME)/schema/core-plexil.xsd $*.plx
	$(RM) $*.epx*

dust:
	$(RM) *.epx *.last lib/*.epx lib/*.last

clean: dust
	$(RM) *.plx lib/*.plx

cleaner: clean
	$(RM) *~ lib/*~

# EOF
