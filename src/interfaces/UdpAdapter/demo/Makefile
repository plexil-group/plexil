# -*- Mode: Makefile -*-
# File: plexil/examples/multi-exec/agents/makefile
# Note: makefile for the test multi-agent applications

ifeq ($(shell uname),Darwin)
  DYLIB=dylib
else
  DYLIB=so
endif

OPTS= -no-ipc #-v -x -vx -d

include $(PLEXIL_HOME)/makeinclude/generic-plexil.make

demo: ../UdpAdapter.$(DYLIB)
	@echo ' '
	$(RUN_AGENTS) $(OPTS) test-recv -c udp test-send -c udp

../UdpAdapter.$(DYLIB):
	$(MAKE) -C ../

.PHONY: demo

# EOF
